#include "mainwindow.h"

#include <QCompleter>
#include <QDebug>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QRegularExpression>
#include <QUndoView>
#include <QWheelEvent>

#include "dialoglabels.h"
#include "loadimganndialog.h"
#include "ui_mainwindow.h"
#include "utils.h"

extern Helper globalHelper;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setUp();
  Helper::InitFonts(font());
  Helper::InitSupportedImageFormats();
  Helper::setImageCanvas(&m_imageCanvas);
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

  connect(m_imageCanvas.undoStack(), &QUndoStack::cleanChanged, this,
          &MainWindow::on_NeedSaveChangeUndo);

  connect(&m_timer, &QTimer::timeout, this, &MainWindow::on_timeout);
  m_timer.start(1000);

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
  m_imageCanvas.prepareForNewBBox(ui->comboBoxActiveLabel->currentText());
}

void MainWindow::on_pbLoadImgAnn_clicked() {
  LoadImgAnnDialog dlg;

  dlg.setImgAndAnnFolders(m_annImgManager.imgFolder(),
                          m_annImgManager.annFolder());
  if (dlg.exec() != QDialog::Accepted) return;

  loadImagesAndAnnotations(dlg.imgFolder(), dlg.annFolder());
}

void MainWindow::on_saveLocalChanges_triggered() {
  auto ann = m_imageCanvas.annotations();

  // image label, description and tags
  ann.label = ui->comboBoxImgLabel->currentText();
  ann.description = ui->pTextImgDescription->toPlainText();
  for (int i = 0; i < ui->listWidgetTags->count(); ++i) {
    ann.tags.append(ui->listWidgetTags->item(i)->text());
  }

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

void MainWindow::on_actionShow_Hide_Labels_triggered() {
  const bool w = !m_imageCanvas.showLabels();
  m_imageCanvas.showLabels(w);
}

void MainWindow::on_actionNew_Point_triggered() {
  m_imageCanvas.prepareForNewPoint(ui->comboBoxActiveLabel->currentText());
}

void MainWindow::on_NeedSaveChangeUndo(bool enable) {
  if (m_needToSaveNotUndo) return;
  ui->saveLocalChanges->setEnabled(!enable);
}

void MainWindow::on_NeedSaveChange() {
  ui->saveLocalChanges->setEnabled(m_needToSaveNotUndo = true);
}

void MainWindow::on_listViewImgNames_clicked(const QModelIndex &index) {
  if (index == m_current_index) return;
  if (ui->saveLocalChanges->isEnabled()) {
    if (ui->checkBoxAutoSave->isChecked()) {
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
  auto image = m_annImgManager.image(image_id);
  auto ann = m_annImgManager.annotations(image_id);
  m_imageCanvas.reset(image, image_id);
  m_imageCanvas.addAnnotations(ann);

  // label
  addNewUniqueItem(ui->comboBoxImgLabel, ann.label, true);

  // description
  ui->pTextImgDescription->setPlainText(ann.description);

  // tags
  ui->listWidgetTags->clear();
  for (auto &tag : ann.tags) {
    auto item = new QListWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(tag);
    addNewUniqueItem(ui->comboBoxTag, tag, false);
    ui->listWidgetTags->addItem(item);
  }
  on_actionFit_Into_View_triggered();
  displayImageInfo();

  ui->saveLocalChanges->setEnabled(false);
  m_needToSaveNotUndo = false;
  m_current_index = index;
  ui->listViewImgNames->setCurrentIndex(m_current_index);
}

void MainWindow::on_actionAdd_New_Line_triggered() {
  m_imageCanvas.prepareForNewLine(ui->comboBoxActiveLabel->currentText());
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
  if (Helper::m_labelsUpdated) {
    QString currLabel = ui->comboBoxActiveLabel->currentText();
    ui->comboBoxActiveLabel->clear();
    ui->comboBoxActiveLabel->addItems(Helper::currentLabels());
    if (currLabel.isEmpty()) {
      ui->comboBoxActiveLabel->setCurrentIndex(0);
    } else {
      ui->comboBoxActiveLabel->setCurrentText(currLabel);
    }
    Helper::m_labelsUpdated = false;
  }
}

void MainWindow::on_tBAdd_clicked() {
  QString currTag = ui->comboBoxTag->currentText();
  if (currTag.isEmpty()) return;

  auto item = new QListWidgetItem();
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  item->setText(currTag);
  auto idx = ui->listWidgetTags->currentIndex();
  if (idx.isValid()) {
    ui->listWidgetTags->insertItem(idx.row() + 1, item);
  } else {
    ui->listWidgetTags->addItem(item);
  }
  ui->comboBoxTag->setCurrentIndex(0);
  on_NeedSaveChange();
}

void MainWindow::on_tBRemove_clicked() {
  auto idx = ui->listWidgetTags->currentIndex();
  if (idx.isValid()) {
    delete ui->listWidgetTags->takeItem(idx.row());
    on_NeedSaveChange();
  }
}

void MainWindow::on_pTextImgDescription_textChanged() { on_NeedSaveChange(); }

void MainWindow::updateSettings() {
  Helper::kPointRadius = ui->doubleSpinBoxPtRadius->value();
  Helper::kFontPixelSize = ui->spinBoxLabelPixSize->value();
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

void MainWindow::on_toolButtonAddItemLabels_clicked() {
  DialogLabels dlb(this, "ITEM'S LABELS");
  dlb.setLabels(getLabelsFromComboBox(ui->comboBoxActiveLabel));
  if (dlb.exec() == QDialog::Accepted) {
      Helper::clearLabels();
      Helper::registerNewLabels(dlb.getLabels());
  }
}

void MainWindow::on_toolButtonAddTags_clicked() {
  DialogLabels dlb(this, "TAGS");
  QStringList tags = getLabelsFromComboBox(ui->comboBoxTag);
  tags.removeFirst();
  dlb.setLabels(tags);
  if (dlb.exec() == QDialog::Accepted) {
    tags = dlb.getLabels();
    tags.push_front("");
    ui->comboBoxTag->clear();
    ui->comboBoxTag->addItems(tags);
  }
}

void MainWindow::on_toolButtonAddImgLabels_clicked() {
  DialogLabels dlb(this, "IMAGE'S LABELS");
  QStringList labels = getLabelsFromComboBox(ui->comboBoxImgLabel);
  dlb.setLabels(labels);
  if (dlb.exec() == QDialog::Accepted) {
    labels = dlb.getLabels();
    ui->comboBoxImgLabel->clear();
    ui->comboBoxImgLabel->addItems(labels);
  }
}

void MainWindow::on_actionSave_project_triggered() {
  const QString filePath =
      QFileDialog::getSaveFileName(this, "Save project file");
  if (filePath.isEmpty()) return;

  QJsonObject root;
  root.insert(QString("ann_folder"), m_annImgManager.annFolder());
  root.insert(QString("img_folder"), m_annImgManager.imgFolder());

  // tags
  QJsonArray array_lbs;
  for (int i = 0; i < ui->comboBoxTag->count(); ++i) {
    QString tag = ui->comboBoxTag->itemText(i);
    if (!tag.isEmpty()) array_lbs.append(tag);
  }
  root[QString("tags")] = array_lbs;

  // obj labes
  array_lbs = {};
  for (int i = 0; i < ui->comboBoxActiveLabel->count(); ++i) {
    QString tag = ui->comboBoxActiveLabel->itemText(i);
    if (!tag.isEmpty()) array_lbs.append(tag);
  }
  root[QString("obj_labels")] = array_lbs;

  // img labes
  array_lbs = {};
  for (int i = 0; i < ui->comboBoxImgLabel->count(); ++i) {
    QString tag = ui->comboBoxImgLabel->itemText(i);
    if (!tag.isEmpty()) array_lbs.append(tag);
  }
  root[QString("img_labels")] = array_lbs;

  const QByteArray out = QJsonDocument(root).toJson();
  QFile ofile(filePath);
  if (!ofile.open(QFile::WriteOnly)) {
    qDebug() << "Failed writing project file: " << filePath;
    return;
  }
  ofile.write(out);
}

void MainWindow::loadImagesAndAnnotations(const QString &annImg,
                                          const QString &annFolder) {
  // clean labels
  Helper::clearLabels();
  m_annImgManager.reset(annImg, annFolder);
  m_imageListModel.setStringList(m_annImgManager.imageIds());
  ui->saveLocalChanges->setEnabled(false);

  QModelIndex tmp = m_imageListModel.indexAtRow(0);
  ui->listViewImgNames->setCurrentIndex(tmp);
  m_current_index = QModelIndex();
  on_listViewImgNames_clicked(tmp);
  ui->actionNext->setEnabled(true);
  ui->actionPrevious->setEnabled(true);
}

void MainWindow::on_actionLoad_project_triggered() {
  const QString filePath =
      QFileDialog::getOpenFileName(this, "Load project file");
  if (filePath.isEmpty()) return;

  QFile ifile(filePath);
  if (!ifile.open(QFile::ReadOnly)) {
    qDebug() << "Failed reading annotations file: " << filePath;
    return;
  }
  const QByteArray json = ifile.readAll();

  QJsonDocument doc = QJsonDocument::fromJson(json);
  QJsonObject root = doc.object();

  // tags
  QStringList lbs;
  for (const auto &lb : root["tags"].toArray()) {
    lbs << lb.toString();
  }
  ui->comboBoxTag->clear();
  lbs.push_front("");
  ui->comboBoxTag->addItems(lbs);

  // object labels
  lbs.clear();
  for (const auto &lb : root["obj_labels"].toArray()) {
    lbs << lb.toString();
  }
  ui->comboBoxActiveLabel->clear();
  Helper::registerNewLabels(lbs);
  // ui->comboBoxActiveLabel->addItems(lbs);

  // img labels
  lbs.clear();
  for (const auto &lb : root["img_labels"].toArray()) {
    lbs << lb.toString();
  }
  ui->comboBoxImgLabel->clear();
  ui->comboBoxImgLabel->addItems(lbs);

  const QString imgFolder = root[QString("img_folder")].toString();
  const QString annFolder = root[QString("ann_folder")].toString();
  loadImagesAndAnnotations(imgFolder, annFolder);
}
