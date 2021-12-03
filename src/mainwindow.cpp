#include "mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QWheelEvent>

#include "loadimganndialog.h"
#include "ui_mainwindow.h"
#include "utils.h"

extern Helper globalHelper;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  Helper::InitFonts(font());
  setUp();
}

MainWindow::~MainWindow() {
  m_timer.stop();
  delete ui;
}

void MainWindow::setUp() {
  ui->bboxEditor->setScene(&m_imageCanvas);
  ui->listViewImgNames->setModel(&m_imageListModel);

  connect(&m_imageCanvas, &ImageCanvas::selectionChanged, this,
          &MainWindow::selectionChangedOnImageCanvas);

  connect(&m_imageCanvas, &ImageCanvas::needSaveChanges, this,
          &MainWindow::onNeedSaveChange);
  connect(&m_timer, &QTimer::timeout, this, &MainWindow::on_timeout);
  m_timer.start(1000);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  ui->tool_panel1_->setMaximumWidth(this->width() / 4);
}

void MainWindow::displayImageInfo() {}

void MainWindow::selectionChangedOnImageCanvas() {
  const auto selected_items = m_imageCanvas.selectedItems();

  if (selected_items.empty())
    m_currentItem = nullptr;
  else if (selected_items.size() == 1)
    m_currentItem = selected_items[0];
  else {
    if (m_currentItem == selected_items[0]) {
      selected_items[0]->setSelected(false);
      m_currentItem = selected_items[1];
    } else {
      selected_items[1]->setSelected(false);
      m_currentItem = selected_items[0];
    }
  }
}

void MainWindow::on_addNewBbox_triggered() {
  m_imageCanvas.prepareForNewBBox(ui->comboBoxActiveLabel->currentText());
}

void MainWindow::on_pbLoadImgAnn_clicked() {
  LoadImgAnnDialog dlg;

  dlg.setImgAndAnnFolders(m_annImgManager.imgFolder(),
                          m_annImgManager.annFolder());
  if (dlg.exec() != QDialog::Accepted)
    return;

  m_annImgManager.reset(dlg.imgFolder(), dlg.annFolder());
  m_imageListModel.setStringList(m_annImgManager.imageIds());
  ui->saveLocalChanges->setEnabled(false);

  QModelIndex tmp = m_imageListModel.indexAtRow(0);
  ui->listViewImgNames->setCurrentIndex(tmp);
  on_listViewImgNames_clicked(tmp);
  ui->actionNext->setEnabled(true);
  ui->actionPrevious->setEnabled(true);
}

void MainWindow::on_saveLocalChanges_triggered() {
  auto ann = m_imageCanvas.annotations();

  // description and tags
  ann.description = ui->pTextImgDescription->toPlainText();
  for (int i = 0; i < ui->listWidgetTags->count(); ++i) {
    ann.tags.append(ui->listWidgetTags->item(i)->text());
  }

  m_annImgManager.saveAnnotations(ann.image_name, ann);
  ui->saveLocalChanges->setEnabled(false);
}

void MainWindow::on_removeBbox_triggered() {
  m_imageCanvas.removeUnlockedItems();
}

void MainWindow::on_actionShowBboxes_triggered() {
  static bool vis = false;
  if (vis)
    m_imageCanvas.showBoundingBoxes();
  else
    m_imageCanvas.hideBoundingBoxes();
  vis = !vis;
}

void MainWindow::on_actionShow_Hide_Labels_triggered() {
  const bool w = !m_imageCanvas.showLabels();
  m_imageCanvas.showLabels(w);
}

void MainWindow::on_actionNew_Point_triggered() {
  m_imageCanvas.prepareForNewPoint(ui->comboBoxActiveLabel->currentText());
}

void MainWindow::onNeedSaveChange() { ui->saveLocalChanges->setEnabled(true); }

void MainWindow::on_listViewImgNames_clicked(const QModelIndex &index) {
  if (index == m_current_index)
    return;
  if (ui->saveLocalChanges->isEnabled()) {
    if (ui->checkBoxAutoSave->isChecked()) {
      on_saveLocalChanges_triggered();
    } else {
      QMessageBox::StandardButton ex = QMessageBox::warning(
          this, "Unsaved changes", "Do you want to saved changes?",
          QMessageBox::StandardButtons(
              {QMessageBox::Save, QMessageBox::Ignore}));
      switch (ex) {
      case QMessageBox::Save:
        on_saveLocalChanges_triggered();
        break;
      default:
        break;
      }
    }
  }

  const QString image_id = m_imageListModel.originalText(index);
  auto image = m_annImgManager.image(image_id);
  auto ann = m_annImgManager.annotations(image_id);
  m_imageCanvas.reset(image, image_id);

  m_imageCanvas.addAnnotations(ann);
  // description and tags
  ui->pTextImgDescription->setPlainText(ann.description);
  ui->listWidgetTags->clear();
  for (auto &tag : ann.tags) {
    auto item = new QListWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(tag);
    ui->listWidgetTags->addItem(item);
  }

  ui->bboxEditor->resetTransform();
  ui->bboxEditor->scale(1.0, 1.0);
  displayImageInfo();
  ui->saveLocalChanges->setEnabled(false);
  m_current_index = index;
}

void MainWindow::on_actionAdd_New_Line_triggered() {
  m_imageCanvas.prepareForNewLine(ui->comboBoxActiveLabel->currentText());
}

void MainWindow::on_actionNext_triggered() {
  int N = m_imageListModel.rowCount();
  const int row = (m_current_index.row() + 1) % N;
  const QModelIndex next_index = m_imageListModel.indexAtRow(row);
  ui->listViewImgNames->setCurrentIndex(next_index);
  on_listViewImgNames_clicked(next_index);
}

void MainWindow::on_actionPrevious_triggered() {
  int N = m_imageListModel.rowCount();
  int row = N - 1;
  if (m_current_index.row() != 0) {
    row = (m_current_index.row() - 1) % N;
  }
  const QModelIndex next_index = m_imageListModel.indexAtRow(row);
  ui->listViewImgNames->setCurrentIndex(next_index);
  on_listViewImgNames_clicked(next_index);
}

void MainWindow::on_actionAdd_New_Polygon_triggered() {
  m_imageCanvas.prepareForNewPolygon(ui->comboBoxActiveLabel->currentText());
}

void MainWindow::on_actionzoom100_triggered() {
  ui->bboxEditor->resetTransform();
  ui->bboxEditor->scale(1.0, 1.0);
  Helper::setScale(1.0);
  m_imageCanvas.helperParametersChanged();
}

void MainWindow::on_actionGrid_triggered(bool checked) {
  m_imageCanvas.setShowGrid(!m_imageCanvas.showGrid());
}

void MainWindow::on_timeout() {
  //  if (Helper::m_labelsUpdated) {
  //    QString currLabel = ui->comboBoxActiveLabel->currentText();
  //    ui->comboBoxActiveLabel->clear();
  //    ui->comboBoxActiveLabel->addItems(Helper::currentLabels());
  //    if (currLabel.isEmpty()) {
  //      ui->comboBoxActiveLabel->setCurrentIndex(0);
  //    } else {
  //      ui->comboBoxActiveLabel->setCurrentText(currLabel);
  //    }
  //    Helper::m_labelsUpdated = false;
  //  }
}

void MainWindow::on_tBAdd_clicked() {
  QString currTag = ui->comboBoxTag->currentText();
  if (currTag.isEmpty())
    return;

  auto item = new QListWidgetItem();
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  item->setText(currTag);
  auto idx = ui->listWidgetTags->currentIndex();
  if (idx.isValid()) {
    ui->listWidgetTags->insertItem(idx.row() + 1, item);
  } else {
    ui->listWidgetTags->addItem(item);
  }
  onNeedSaveChange();
}

void MainWindow::on_tBRemove_clicked() {
  auto idx = ui->listWidgetTags->currentIndex();
  if (idx.isValid()) {
    delete ui->listWidgetTags->takeItem(idx.row());
    onNeedSaveChange();
  }
}

void MainWindow::on_pTextImgDescription_textChanged() { onNeedSaveChange(); }

void MainWindow::on_pushButtonUpdate_clicked() {
  Helper::kPointRadius = ui->doubleSpinBoxPtRadius->value();
  Helper::kFontPixelSize = ui->spinBoxLabelPixSize->value();
  Helper::InitFonts(font());
  m_imageCanvas.helperParametersChanged();
}

void MainWindow::on_doubleSpinBoxPtRadius_valueChanged(double arg1) {
  on_pushButtonUpdate_clicked();
}

void MainWindow::on_spinBoxLabelPixSize_valueChanged(int arg1) {
  on_pushButtonUpdate_clicked();
}
