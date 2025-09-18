#include "mainwindow.h"

#include <QCompleter>
#include <QDebug>
#include <QFileDialog>
#include <QFuture>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPromise>
#include <QRegularExpression>
#include <QUndoView>
#include <QWheelEvent>

#include "dialoglabels.h"
#include "import_export/coco.h"
#include "import_export/native.h"
#include "import_export/yolo.h"
#include "labeltreemodel.h"
#include "loadimganndialog.h"
#include "ui_mainwindow.h"
#include "utils.h"

extern Helper globalHelper;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  Helper::InitFonts(font());
  Helper::InitSupportedImageFormats();
  Helper::setImageCanvas(&m_imageCanvas);
  Helper::labelTreeModel = new LabelTreeModel(this);
  ui->treeViewLabels->setModel(Helper::labelTreeModel);

  setUp();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setUp() {
  ui->bboxEditor->setScene(&m_imageCanvas);
  ui->listViewImgNames->setModel(&m_imageListModel);

  connect(&m_imageCanvas, &ImageCanvas::selectionChanged, this,
          &MainWindow::selectionChangedOnImageCanvas);

  connect(m_imageCanvas.undoStack(), &QUndoStack::cleanChanged, this,
          &MainWindow::on_NeedSaveChangeUndo);

  auto undoAction = m_imageCanvas.undoStack()->createUndoAction(this);
  undoAction->setIcon(QIcon(":/icons/icons/edit-undo.svg"));
  undoAction->setShortcut(QKeySequence(tr("Ctrl+Z")));
  ui->mainToolBar->addAction(undoAction);

  auto redoAction = m_imageCanvas.undoStack()->createRedoAction(this);
  redoAction->setIcon(QIcon(":/icons/icons/edit-redo.svg"));
  redoAction->setShortcut(QKeySequence(tr("Ctrl+Shift+Z")));
  ui->mainToolBar->addAction(redoAction);

  ui->undoView->setStack(m_imageCanvas.undoStack());
  ui->saveLocalChanges->setEnabled(false);

  ui->statusBar->addWidget(m_displayLabel = new QLabel);

  ui->dockWidgetEditHistorial->close();
  ui->dockWidgetSettings->close();

  connect(Helper::labelTreeModel,
          SIGNAL(labelEnableChanged(Helper::CustomItemType, QString, bool)),
          &m_imageCanvas,
          SLOT(onLabelEnableChanged(Helper::CustomItemType, QString, bool)));
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  ui->dockWidgetL->setMaximumWidth(this->width() / 4);
}

void MainWindow::displayImageInfo() {}

void MainWindow::addNewUniqueItem(QComboBox *cbox, const QString &label,
                                  bool selected) {
  const int n = cbox->count();
  int i;
  for (i = 0; i < n; ++i) {
    if (cbox->itemText(i) == label) {
      if (selected) cbox->setCurrentIndex(i);
      return;
    }
  }
  cbox->addItem(label);
  if (selected) cbox->setCurrentIndex(i);
}

QStringList MainWindow::getLabelsFromComboBox(QComboBox *cbox) {
  QStringList labels;
  for (int i = 0; i < cbox->count(); ++i) {
    labels << cbox->itemText(i);
  }
  return labels;
}

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
  m_imageCanvas.prepareForNewBBox("");
}

void MainWindow::on_pbLoadImgAnn_clicked() {
  ui->listViewImgNames->setEnabled(false);

  QSettings st;
  st.beginGroup("recent_image_annotation_folders");
  const auto keys = st.childKeys();  // O(n) listing once
  QList<std::tuple<QString, QString, qint64>> recents;
  recents.reserve(keys.size());
  for (const auto &k : keys) {
    QString image_path, anno_path;
    qint64 dti = 0;

    if (unpackProjectTuple(st.value(k), image_path, anno_path, dti)) {
      recents.emplace_back(image_path, anno_path, dti);
    }
  }
  st.endGroup();

  std::sort(recents.begin(), recents.end(),
            [](auto &&x, auto &&y) { return std::get<2>(x) < std::get<2>(y); });

  LoadImgAnnDialog dlg;
  dlg.setImgAndAnnFolders(m_annImgManager.imgFolder(),
                          m_annImgManager.annFolder(), recents);
  if (dlg.exec() != QDialog::Accepted) {
    ui->listViewImgNames->setEnabled(true);
    return;
  }

  bool ok = loadImagesAndAnnotations(dlg.imgFolder(), dlg.annFolder());
  ui->listViewImgNames->setEnabled(ok);

  if (ok) {
    st.beginGroup("recent_image_annotation_folders");
    auto key = makeProjectKey(m_annImgManager.imgFolder(),
                              m_annImgManager.annFolder());
    auto value = packProjectTuple(m_annImgManager.imgFolder(),
                                  m_annImgManager.annFolder(),
                                  Helper::seconsToYear5000());
    st.setValue(key, value);
    st.endGroup();
    st.sync();
  }
}

void MainWindow::on_saveLocalChanges_triggered() {
  Annotations ann = m_imageCanvas.annotations();

  // image label, description and tags
  ann.label = ui->comboBoxImgLabel->currentText();
  ann.description = ui->pTextImgDescription->toPlainText();

  m_annImgManager.saveAnnotations(ann.image_name, ann);
  ui->saveLocalChanges->setEnabled(false);
  m_needToSaveNotUndo = false;
  m_imageCanvas.undoStack()->setClean();
}

void MainWindow::on_actionShowBboxes_triggered() {
  static bool vis = false;
  if (vis)
    m_imageCanvas.showBoundingBoxes();
  else
    m_imageCanvas.hideBoundingBoxes();
  vis = !vis;
}

void MainWindow::on_actionNew_Point_triggered() {
  m_imageCanvas.prepareForNewPoint("");
}

void MainWindow::on_NeedSaveChangeUndo(bool enable) {
  if (m_needToSaveNotUndo) return;
  ui->saveLocalChanges->setEnabled(!enable);
}

void MainWindow::on_NeedSaveChange() {
  ui->saveLocalChanges->setEnabled(m_needToSaveNotUndo = true);
}

///////////////////////////////////////////////////////
///////  LOAD ANNOTATIONS TO CANVAS                  //
///////////////////////////////////////////////////////
void MainWindow::on_listViewImgNames_clicked(const QModelIndex &index) {
  if (index == m_current_index) return;

  if (ui->saveLocalChanges->isEnabled()) {
    if (ui->actionAuto_Save->isChecked()) {
      on_saveLocalChanges_triggered();
    } else {
      QMessageBox::StandardButton ex = QMessageBox::warning(
          this, "Unsaved changes", "Do you want to saved changes?",
          QMessageBox::StandardButtons(
              {QMessageBox::Save, QMessageBox::Ignore, QMessageBox::Cancel}));
      switch (ex) {
        case QMessageBox::Save:
          on_saveLocalChanges_triggered();
          break;
        case QMessageBox::Cancel:
          ui->listViewImgNames->setCurrentIndex(m_current_index);
          return;
          break;
        case QMessageBox::Ignore:
          break;
        default:
          break;
      }
    }
  }

  const QString image_id = m_imageListModel.originalText(index);
  const QImage &image = m_annImgManager.image(image_id);
  bool fromCached;
  const Annotations &ann = m_annImgManager.annotations(image_id, &fromCached);

  // WHEN ANNOTATIONS COME FROM THE CACHE THE TAGS ARE ALREADY REGISTERED
  if (!fromCached) {
    Helper::labelTreeModel->populateLabelsFromAnnotations(ann);
  }

  m_imageCanvas.reset(image, image_id);
  m_imageCanvas.addAnnotations(ann);

  // label
  addNewUniqueItem(ui->comboBoxImgLabel, ann.label, true);

  // description
  ui->pTextImgDescription->setPlainText(ann.description);

  on_actionFit_Into_View_triggered();
  displayImageInfo();

  ui->saveLocalChanges->setEnabled(false);
  m_needToSaveNotUndo = false;
  m_current_index = index;
  ui->listViewImgNames->setCurrentIndex(m_current_index);
}

void MainWindow::on_actionAdd_New_Line_triggered() {
  m_imageCanvas.prepareForNewLine("");
}

void MainWindow::on_actionNext_triggered() {
  int N = m_imageListModel.rowCount();
  const int row = (m_current_index.row() + 1) % N;
  const QModelIndex next_index = m_imageListModel.indexAtRow(row);
  on_listViewImgNames_clicked(next_index);
}

void MainWindow::on_actionPrevious_triggered() {
  int N = m_imageListModel.rowCount();
  int row = N - 1;
  if (m_current_index.row() != 0) {
    row = (m_current_index.row() - 1) % N;
  }
  const QModelIndex next_index = m_imageListModel.indexAtRow(row);
  on_listViewImgNames_clicked(next_index);
}

void MainWindow::on_actionAdd_New_Polygon_triggered() {
  m_imageCanvas.prepareForNewPolygon("");
}

void MainWindow::on_actionzoom100_triggered() {
  ui->bboxEditor->resetTransform();
  ui->bboxEditor->scale(1.0, 1.0);
  Helper::setScale(1.0);
  m_imageCanvas.helperParametersChanged();
}

void MainWindow::on_actionGrid_triggered(bool checked) {
  m_imageCanvas.setShowGrid(checked);
}

void MainWindow::on_pTextImgDescription_textChanged() { on_NeedSaveChange(); }

void MainWindow::updateSettings() {
  Helper::kPointRadius = ui->doubleSpinBoxPtRadius->value();
  Helper::kFontPixelSize = ui->spinBoxLabelPixSize->value();
  Helper::kLineWidth = qMax(1.0, ui->doubleSpinBoxPtRadius->value() / 3);
  Helper::setScale(Helper::kInvScaleFactor);
  m_imageCanvas.helperParametersChanged();
}

void MainWindow::on_doubleSpinBoxPtRadius_valueChanged(double arg1) {
  updateSettings();
}

void MainWindow::on_spinBoxLabelPixSize_valueChanged(int arg1) {
  updateSettings();
}

void MainWindow::on_actionFit_Into_View_triggered() {
  ui->bboxEditor->fitInView(ui->bboxEditor->sceneRect(), Qt::KeepAspectRatio);
  Helper::setScale(1.0 / ui->bboxEditor->transform().m11());
  m_imageCanvas.helperParametersChanged();
}

void MainWindow::on_comboBoxImgLabel_currentTextChanged(const QString &arg1) {
  on_NeedSaveChange();
}

void MainWindow::on_toolButtonAddImgLabels_clicked() {
  DialogLabels dlb(this, "Image's labels");
  QStringList labels = getLabelsFromComboBox(ui->comboBoxImgLabel);
  dlb.setLabels(labels);
  if (dlb.exec() == QDialog::Accepted) {
    labels = dlb.getLabels();
    ui->comboBoxImgLabel->clear();
    ui->comboBoxImgLabel->addItems(labels);
  }
}

/**
 * @brief MainWindow::loadImagesAndAnnotations Load a new project
 * @param annImg
 * @param annFolder
 * @return true|false
 */
bool MainWindow::loadImagesAndAnnotations(const QString &annImg,
                                          const QString &annFolder) {
  // clean labels
  m_annImgManager.reset(annImg, annFolder);
  if (m_annImgManager.annotationsCount() == 0) {
    QMessageBox::information(
        this, "Not valid images folder",
        "Not images found in folder: " + m_annImgManager.imgFolder() +
            "\nPlease, select another folder.");
    return false;
  }

  Helper::labelTreeModel->clear();
  m_imageListModel.setStringList(m_annImgManager.imageIds());
  ui->saveLocalChanges->setEnabled(false);

  QModelIndex tmp = m_imageListModel.indexAtRow(0);
  ui->listViewImgNames->setCurrentIndex(tmp);
  m_current_index = QModelIndex();
  on_listViewImgNames_clicked(tmp);
  ui->actionNext->setEnabled(true);
  ui->actionPrevious->setEnabled(true);
  ui->mainToolBar->setToolTip(m_annImgManager.imgFolder());
  m_displayLabel->setText(u8"\U0001F5BC " + m_annImgManager.imgFolder() +
                          u8" \U0001F4DD " + m_annImgManager.annFolder());

  return true;
}

void MainWindow::on_actionAdd_New_LineStrip_triggered() {
  m_imageCanvas.prepareForNewLineStrip("");
}

void MainWindow::on_actionAdd_Circle_Item_triggered() {
  m_imageCanvas.prepareForNewCircle("");
}

void MainWindow::on_actionExportCoco_triggered() {
  const QString fileName = QFileDialog::getSaveFileName(this, "Coco JSON file");
  if (fileName.isEmpty()) return;

  QProgressDialog progress("Exporting to COCO format...", "Cancel", 0, 100,
                           this);
  progress.setMinimumDuration(0);
  progress.setWindowModality(Qt::WindowModal);
  exportCOCOAnnotationsTask(m_annImgManager, fileName, progress, true, true);
}

void MainWindow::on_actionEdit_historial_triggered(bool checked) {
  if (checked) {
    ui->dockWidgetEditHistorial->show();
  } else {
    ui->dockWidgetEditHistorial->close();
  }
}

void MainWindow::on_actionSettings_triggered(bool checked) {
  if (checked) {
    ui->dockWidgetSettings->show();
  } else {
    ui->dockWidgetSettings->close();
  }
}

void MainWindow::on_actionShow_Hide_Labels_triggered(bool checked) {
  m_imageCanvas.setShowLabels(checked);
}

void MainWindow::on_actionExportYolo_triggered() {
  const QString outDir = QFileDialog::getExistingDirectory(
      this, "Select folder for YOLO annotations");
  if (outDir.isEmpty()) return;

  QProgressDialog progress("Exporting to YOLO format...", "Cancel", 0, 100,
                           this);
  progress.setWindowModality(Qt::WindowModal);
  exportProjectToYOLO(m_annImgManager, outDir, progress, true, true);
}

void MainWindow::on_actionNative_triggered() {
  const QString fileName =
      QFileDialog::getSaveFileName(this, "Native JSON file");
  if (fileName.isEmpty()) return;

  QProgressDialog progress("Exporting to a single JSON file...", "Cancel", 0,
                           100, this);
  progress.setWindowModality(Qt::WindowModal);
  exportProjectToJSON(m_annImgManager, fileName, progress);
}
