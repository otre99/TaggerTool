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
#include "exportdialog.h"
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
  setUp();
}

MainWindow::~MainWindow() {
  m_imageCanvas.undoStack()->disconnect(this);
  delete ui;
}

void MainWindow::setUp() {
  // init some global parameters
  Helper::InitFonts(font());
  Helper::InitSupportedImageFormats();

  // make images canvas global
  Helper::setImageCanvas(&m_imageCanvas);

  // make labels model global
  Helper::labelTreeModel = new LabelTreeModel(this);
  ui->treeViewLabels->setModel(Helper::labelTreeModel);

  // set up image canvas
  ui->bboxEditor->setScene(&m_imageCanvas);
  ui->listViewImgNames->setModel(&m_imageListModel);

  // set up Undo Stack
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

  // set up status bar
  ui->statusBar->addWidget(m_displayLabel = new QLabel);

  ui->dockWidgetEditHistorial->close();
  ui->dockWidgetSettings->close();

  // show/hide item based on labels enable/disable
  connect(Helper::labelTreeModel,
          SIGNAL(labelEnableChanged(Helper::CustomItemType, QString, bool)),
          &m_imageCanvas,
          SLOT(onLabelEnableChanged(Helper::CustomItemType, QString, bool)));

  // extra docker windos
  connect(ui->actionSettings, &QAction::triggered, ui->dockWidgetSettings,
          &QDockWidget::setVisible);
  connect(ui->actionEdit_historial, &QAction::triggered,
          ui->dockWidgetEditHistorial, &QDockWidget::setVisible);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  ui->dockWidgetL->setMaximumWidth(this->width() / 4);
}

void MainWindow::closeEvent(QCloseEvent *ev) {
  if (ui->saveLocalChanges->isEnabled()) {
    const auto choice = QMessageBox::warning(
        this, tr("Unsaved changes"), tr("Save changes before closing?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);

    switch (choice) {
      case QMessageBox::Save: {
        on_saveLocalChanges_triggered();
        ev->accept();
        return;
      }
      case QMessageBox::Discard:
        ev->accept();
        return;
      case QMessageBox::Cancel:
      default:
        ev->ignore();
        return;
    }
  }
  ev->accept();
}

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

void MainWindow::on_addNewBbox_triggered() {
  m_imageCanvas.prepareForNewBBox("");
}

void MainWindow::on_pbLoadImgAnn_clicked() {
  ui->listViewImgNames->setEnabled(false);

  LoadImgAnnDialog dlg;
  dlg.setImgAndAnnFolders(m_annImgManager.imgFolder(),
                          m_annImgManager.annFolder(),
                          m_recentProjectsManager.recentEntries());
  if (dlg.exec() != QDialog::Accepted) {
    ui->listViewImgNames->setEnabled(true);
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  bool ok = loadImagesAndAnnotations(dlg.imgFolder(), dlg.annFolder());
  ui->listViewImgNames->setEnabled(ok);

  if (ok) {
    m_recentProjectsManager.updateEntry(dlg.imgFolder(), dlg.annFolder());
  }
  QApplication::restoreOverrideCursor();
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
  // TODO(otre99): fix this
  static bool vis = false;
  if (vis)
    m_imageCanvas.showNonEditableAnnotationItems();
  else
    m_imageCanvas.hideNonEditableAnnotationItems();
  vis = !vis;
}

void MainWindow::on_actionNew_Point_triggered() {
  m_imageCanvas.prepareForNewPoint("");
}

void MainWindow::on_NeedSaveChangeUndo(bool clean) {
  if (m_needToSaveNotUndo) return;
  ui->saveLocalChanges->setEnabled(!clean);
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

  ui->saveLocalChanges->setEnabled(false);
  m_needToSaveNotUndo = false;
  m_current_index = index;
  ui->listViewImgNames->setCurrentIndex(m_current_index);
}

void MainWindow::on_actionAdd_New_Line_triggered() {
  m_imageCanvas.prepareForNewLine("");
}

void MainWindow::on_actionNext_triggered() {
  const int N = m_imageListModel.rowCount();
  if (N == 0) return;
  const int row = (m_current_index.row() + 1) % N;
  const QModelIndex next_index = m_imageListModel.indexAtRow(row);
  on_listViewImgNames_clicked(next_index);
}

void MainWindow::on_actionPrevious_triggered() {
  const int N = m_imageListModel.rowCount();
  if (N == 0) return;
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

bool MainWindow::loadImagesAndAnnotations(const QString &annImg,
                                          const QString &annFolder,
                                          const QString &imageId) {
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
  const QStringList imageFileNames = m_annImgManager.imageIds();
  m_imageListModel.setStringList(imageFileNames);
  ui->saveLocalChanges->setEnabled(false);

  int index = imageFileNames.indexOf(imageId);
  QModelIndex tmp = m_imageListModel.indexAtRow(index);
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
  runExport(ExportOptions::Format::Coco);
}

// void MainWindow::on_actionEdit_historial_triggered(bool checked) {
//   if (checked) {
//     ui->dockWidgetEditHistorial->show();
//   } else {
//     ui->dockWidgetEditHistorial->close();
//   }
// }

// void MainWindow::on_actionSettings_triggered(bool checked) {
//   if (checked) {
//     ui->dockWidgetSettings->show();
//   } else {
//     ui->dockWidgetSettings->close();
//   }
// }

void MainWindow::on_actionShow_Hide_Labels_triggered(bool checked) {
  m_imageCanvas.setShowLabels(checked);
}

void MainWindow::on_actionExportYolo_triggered() {
  runExport(ExportOptions::Format::Yolo);
}

void MainWindow::on_actionNative_triggered() {
  runExport(ExportOptions::Format::Native);
}

/**
 * @brief Ask for the export settings, then run the matching exporter.
 */
void MainWindow::runExport(ExportOptions::Format format) {
  if (m_annImgManager.annotationsCount() == 0) {
    QMessageBox::information(this, tr("Nothing to export"),
                             tr("Load a project first."));
    return;
  }

  ExportDialog dlg(format, &m_annImgManager, this);
  if (dlg.exec() != QDialog::Accepted) return;

  const ExportOptions opt = dlg.options();
  if (opt.outputPath.isEmpty()) return;
  // Remember the choices so the next export of this format starts here.
  opt.save(ExportDialog::settingsGroup(format));

  const QString name = ExportDialog::formatName(format);
  QProgressDialog progress(tr("Exporting to %1...").arg(name), tr("Cancel"), 0,
                           100, this);
  progress.setMinimumDuration(0);
  progress.setWindowModality(Qt::WindowModal);

  bool ok = false;
  switch (format) {
    case ExportOptions::Format::Native:
      ok = exportProjectToJSON(m_annImgManager, opt, progress);
      break;
    case ExportOptions::Format::Coco:
      ok = exportCOCOAnnotationsTask(m_annImgManager, opt, progress);
      break;
    case ExportOptions::Format::Yolo:
      ok = exportProjectToYOLO(m_annImgManager, opt, progress);
      break;
  }
  progress.close();

  if (ok) {
    ui->statusBar->showMessage(
        tr("Export to %1 finished: %2").arg(name, opt.outputPath), 8000);
  } else if (!progress.wasCanceled()) {
    QMessageBox::warning(
        this, tr("Export failed"),
        tr("The %1 export could not be completed. Check that the destination "
           "is writable.")
            .arg(name));
  }
}
