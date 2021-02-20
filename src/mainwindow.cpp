#include "mainwindow.h"
#include "annotationfilter.h"
#include "ui_mainwindow.h"
#include "utils.h"
#include <QDebug>
#include <QFileDialog>
#include <QWheelEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), current_bbox_item_(nullptr) {
  ui->setupUi(this);
  setUp();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setUp() {
  ui->bboxEditor->setScene(&m_imageCanvas);
  ui->listViewImgNames->setModel(&m_imageListModel);

  this->tabifyDockWidget(ui->toolPanel2, ui->tool_panel1_);
  connect(&m_imageCanvas, SIGNAL(BBoxItemToEditor(QGraphicsItem *, int)), this,
          SLOT(DisplayBBoxInfo(QGraphicsItem *, int)));

  ui->mainToolBar->addWidget(ui->toolButtonZoom100);

  connect(&m_imageCanvas, &ImageCanvas::selectionChanged, this,
          &MainWindow::selectionChangedOnImageCanvas);
  connect(this, &MainWindow::exportAnnotations, &m_worker,
          &ExpensiveRoutines::exportAnnotations);

  connect(&m_worker, &ExpensiveRoutines::jobStarted, ui->progressBar,
          &QProgressBar::setVisible);

  connect(&m_worker, &ExpensiveRoutines::progress, ui->progressBar,
          &QProgressBar::setValue);

  connect(&m_worker, &ExpensiveRoutines::jobFinished, ui->progressBar,
          &QProgressBar::setVisible);

  ui->statusBar->addPermanentWidget(ui->progressBar);
  ui->progressBar->setVisible(false);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  ui->tool_panel1_->setMaximumWidth(this->width() / 4);
}

void MainWindow::on_listViewImgNames_activated(const QModelIndex &index) {
  const QString image_id = m_imageListModel.originalText(index);

  // if ( image_id == m_imageCanvas.GetImageId() ) return;

  auto image = m_annImgManager.image(image_id);
  auto ann = m_annImgManager.annotations(image_id);
  m_imageCanvas.reset(image, image_id);
  const QStringList keys = ann.keys();
  for (const auto &key : keys) {
    m_imageCanvas.addBoundingBoxes(ann[key], QStringList() << key);
  }
  ui->bboxEditor->resetTransform();
  ui->bboxEditor->scale(1.0, 1.0);
  displayImageInfo();
}

void MainWindow::on_lineEditImgFilter_editingFinished() {
  QRegExp regexp(ui->lineEditImgFilter->text(), Qt::CaseInsensitive,
                 QRegExp::Wildcard);
  m_imageListModel.setStringList(m_annImgManager.imageIds().filter(regexp));
}

void MainWindow::displayImageInfo() {}

void MainWindow::displayBBoxInfo(QGraphicsItem *newFocusItem, int reason) {
  (void)reason;
  auto *bbox_item = dynamic_cast<BoundingBoxItem *>(newFocusItem);
  if (bbox_item != current_bbox_item_)
    return;

  if (current_bbox_item_) {
    ui->lineEditBboxLabel->setText(current_bbox_item_->label());
    const auto r = current_bbox_item_->boundingBoxCoordinates();
    ui->lineEditBboxX->setText(QString::number(r.x()));
    ui->lineEditBboxW->setText(QString::number(r.width()));
    ui->lineEditBboxY->setText(QString::number(r.y()));
    ui->lineEditBboxH->setText(QString::number(r.height()));
  } else {
    ui->lineEditBboxX->setText("");
    ui->lineEditBboxW->setText("");
    ui->lineEditBboxY->setText("");
    ui->lineEditBboxH->setText("");
  }
}

void MainWindow::selectionChangedOnImageCanvas() {
  const auto selected_items = m_imageCanvas.selectedItems();

  if (selected_items.empty())
    current_bbox_item_ = nullptr;
  else if (selected_items.size() == 1)
    current_bbox_item_ = dynamic_cast<BoundingBoxItem *>(selected_items[0]);
  else {
    if (current_bbox_item_ ==
        dynamic_cast<BoundingBoxItem *>(selected_items[0])) {
      selected_items[0]->setSelected(false);
      current_bbox_item_ = dynamic_cast<BoundingBoxItem *>(selected_items[1]);
    } else {
      selected_items[1]->setSelected(false);
      current_bbox_item_ = dynamic_cast<BoundingBoxItem *>(selected_items[0]);
    }
  }
  displayBBoxInfo(current_bbox_item_, 0);
}

// void MainWindow::on_zoom_spin_box__valueChanged(int arg1)
//{
//    ui->bbox_editor_->resetTransform();
//    ui->bbox_editor_->scale(arg1/100.0, arg1/100.0);
//}

void MainWindow::on_addNewBbox_triggered() {
  m_imageCanvas.prepareForNewBBox();
  m_imageCanvas.showNiceText("Adding new bbox");
}

void MainWindow::on_addLabelToFilterList_clicked() {
  const int rows = ui->tableFilterList->rowCount() + 1;
  ui->tableFilterList->setRowCount(rows);
  ui->tableFilterList->setItem(rows - 1, 0,
                               new QTableWidgetItem(ui->leLabelToAdd->text()));
  ui->tableFilterList->setItem(rows - 1, 1,
                               new QTableWidgetItem(ui->leLabelToAdd->text()));
}

void MainWindow::on_tbLoadImageFolder_clicked() {
  const QString img_folder = QFileDialog::getExistingDirectory(this);
  if (img_folder.isEmpty())
    return;
  ui->leInputImageFolder->setText(img_folder);
}

void MainWindow::on_tbLoadAnnFolder_clicked() {
  const QString ann_folder = QFileDialog::getExistingDirectory(this);
  if (ann_folder.isEmpty())
    return;
  ui->leInputAnnFolder->setText(ann_folder);
}

void MainWindow::on_pbLoadImgAnn_clicked() {
  m_annImgManager.reset(ui->leInputImageFolder->text(),
                        ui->leInputAnnFolder->text(),
                        ui->cbboxAnnFormat->currentText());

  m_imageListModel.setStringList(m_annImgManager.imageIds());
}

void MainWindow::on_removeLabelToFilterList_clicked() {
  const auto ranges = ui->tableFilterList->selectedRanges();
  QVector<int> rows_to_remove;
  for (auto &r : ranges) {
    for (int i = r.topRow(); i <= r.bottomRow(); ++i)
      rows_to_remove.push_back(i);
  }
  std::sort(rows_to_remove.begin(), rows_to_remove.end());
  std::reverse_iterator<QVector<int>::Iterator> r = rows_to_remove.rbegin();
  while (r != rows_to_remove.rend()) {
    ui->tableFilterList->removeRow(*r);
    ++r;
  }
}

void MainWindow::on_pbExport_clicked() {
  QString out_path;
  if (ui->comboBoxLineFormat->currentIndex() == 2)
    out_path = QFileDialog::getSaveFileName();
  else
    out_path = QFileDialog::getExistingDirectory();
  if (out_path.isEmpty())
    return;

  QMap<QString, QString> f;
  for (int i = 0; i < ui->tableFilterList->rowCount(); ++i) {
    const auto key = ui->tableFilterList->item(i, 0)->text();
    const auto value = ui->tableFilterList->item(i, 1)->text();
    f[key] = value;
  }
  AnnotationFilter ann_filter;
  ann_filter.setFilteredLabels(f);

  emit exportAnnotations(&m_annImgManager, ann_filter, out_path,
                         ui->comboBoxLineFormat->currentIndex());
}

void MainWindow::on_saveLocalChanges_triggered() {
  const auto ann = m_imageCanvas.boundingBoxes();
  const QString img_id = m_imageCanvas.imageId();
  const QSize img_size = m_imageCanvas.imageSize();
  m_annImgManager.saveAnnotations(img_id, ann, img_size);
  m_imageCanvas.showNiceText("Saved");
}

void MainWindow::on_removeBbox_triggered() {
  m_imageCanvas.removeSelectedBoundingBoxes();
}

void MainWindow::on_actionShowBboxes_triggered() {
  static bool vis = true;
  if (vis)
    m_imageCanvas.showBoundingBoxes();
  else
    m_imageCanvas.hideBoundingBoxes();
  vis = !vis;
}

void MainWindow::on_toolButtonZoom100_clicked() {
  ui->bboxEditor->resetTransform();
  ui->bboxEditor->scale(1.0, 1.0);
}
