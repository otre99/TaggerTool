#include "exportdialog.h"

#include <QAbstractButton>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QProgressDialog>
#include <QPushButton>
#include <QTableWidgetItem>

#include "ui_exportdialog.h"
#include "utils.h"

namespace {

constexpr int kLabelColumn = 0;
constexpr int kCountColumn = 1;
constexpr int kClassColumn = 2;

QString handlingText(ExportOptions::Handling h) {
  switch (h) {
    case ExportOptions::Handling::Skip:
      return QObject::tr("Do not export");
    case ExportOptions::Handling::Native:
      return QObject::tr("Keep as is");
    case ExportOptions::Handling::AsBBox:
      return QObject::tr("Bounding box");
    case ExportOptions::Handling::AsPolygon:
      return QObject::tr("Polygon");
  }
  return {};
}

}  // namespace

ExportDialog::ExportDialog(ExportOptions::Format format, AnnImgManager *mgr,
                           QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ExportDialog),
      m_format(format),
      m_mgr(mgr) {
  ui->setupUi(this);
  setWindowTitle(tr("Export to %1").arg(formatName(format)));

  populateCombos();
  populateFormatPage();
  scanProject();
  fillLabelTable();

  ExportOptions opt;
  opt.format = m_format;
  // Defaults first, then whatever the previous export of this format used.
  opt.handling[Helper::kBBox] = ExportOptions::Handling::Native;
  opt.handling[Helper::kPolygon] = ExportOptions::Handling::Native;
  if (m_format == ExportOptions::Format::Native) {
    opt.handling[Helper::kLineStrip] = ExportOptions::Handling::Native;
    opt.handling[Helper::kCircle] = ExportOptions::Handling::Native;
    opt.handling[Helper::kPoint] = ExportOptions::Handling::Native;
    opt.handling[Helper::kLine] = ExportOptions::Handling::Native;
  } else {
    // Types the format cannot store are left out until the user opts in.
    opt.handling[Helper::kLineStrip] = ExportOptions::Handling::Skip;
    opt.handling[Helper::kCircle] = ExportOptions::Handling::Skip;
    opt.handling[Helper::kPoint] = ExportOptions::Handling::Skip;
    opt.handling[Helper::kLine] = ExportOptions::Handling::Skip;
  }
  opt.load(settingsGroup(m_format));
  applyOptions(opt);
  // setChecked() above only emits when the state actually changed, so the
  // check box column is put in sync explicitly.
  on_checkBoxFilterLabels_toggled(ui->checkBoxFilterLabels->isChecked());
  on_comboBoxYoloLayout_currentIndexChanged(
      ui->comboBoxYoloLayout->currentIndex());
  updateOkState();
}

ExportDialog::~ExportDialog() { delete ui; }

QString ExportDialog::formatName(ExportOptions::Format format) {
  switch (format) {
    case ExportOptions::Format::Native:
      return QStringLiteral("Native JSON");
    case ExportOptions::Format::Coco:
      return QStringLiteral("COCO");
    case ExportOptions::Format::Yolo:
      return QStringLiteral("YOLO");
  }
  return {};
}

QString ExportDialog::settingsGroup(ExportOptions::Format format) {
  switch (format) {
    case ExportOptions::Format::Native:
      return QStringLiteral("native");
    case ExportOptions::Format::Coco:
      return QStringLiteral("coco");
    case ExportOptions::Format::Yolo:
      return QStringLiteral("yolo");
  }
  return QStringLiteral("native");
}

QComboBox *ExportDialog::comboFor(Helper::CustomItemType type) const {
  switch (type) {
    case Helper::kBBox:
      return ui->comboBoxBBox;
    case Helper::kPolygon:
      return ui->comboBoxPolygon;
    case Helper::kLineStrip:
      return ui->comboBoxLineStrip;
    case Helper::kCircle:
      return ui->comboBoxCircle;
    case Helper::kPoint:
      return ui->comboBoxPoint;
    case Helper::kLine:
      return ui->comboBoxLine;
    default:
      return nullptr;
  }
}

QLabel *ExportDialog::countLabelFor(Helper::CustomItemType type) const {
  switch (type) {
    case Helper::kBBox:
      return ui->labelCountBBox;
    case Helper::kPolygon:
      return ui->labelCountPolygon;
    case Helper::kLineStrip:
      return ui->labelCountLineStrip;
    case Helper::kCircle:
      return ui->labelCountCircle;
    case Helper::kPoint:
      return ui->labelCountPoint;
    case Helper::kLine:
      return ui->labelCountLine;
    default:
      return nullptr;
  }
}

QVector<ExportOptions::Handling> ExportDialog::allowedHandling(
    Helper::CustomItemType type) const {
  using H = ExportOptions::Handling;
  const bool nativeFormat = (m_format == ExportOptions::Format::Native);

  switch (type) {
    case Helper::kBBox:
      // "Keep as is" already is a bounding box, so AsBBox would be redundant.
      return {H::Native, H::AsPolygon, H::Skip};
    case Helper::kPolygon:
      return {H::Native, H::AsBBox, H::Skip};
    case Helper::kLineStrip:
    case Helper::kCircle:
    case Helper::kPoint:
      return nativeFormat
                 ? QVector<H>{H::Native, H::AsBBox, H::AsPolygon, H::Skip}
                 : QVector<H>{H::AsBBox, H::AsPolygon, H::Skip};
    case Helper::kLine:
      // A line has no meaningful closed-polygon form in COCO/YOLO.
      return nativeFormat ? QVector<H>{H::Native, H::AsBBox, H::Skip}
                          : QVector<H>{H::AsBBox, H::Skip};
    default:
      return {H::Skip};
  }
}

void ExportDialog::populateCombos() {
  for (auto type : ExportOptions::allTypes()) {
    QComboBox *combo = comboFor(type);
    if (!combo) continue;
    combo->clear();
    for (auto h : allowedHandling(type)) {
      combo->addItem(handlingText(h), static_cast<int>(h));
    }
  }

  ui->comboBoxYoloLayout->clear();
  ui->comboBoxYoloLayout->addItem(
      tr("labels/ only"),
      static_cast<int>(ExportOptions::YoloLayout::LabelsOnly));
  ui->comboBoxYoloLayout->addItem(
      tr("Full dataset (images/ + labels/)"),
      static_cast<int>(ExportOptions::YoloLayout::FullDataset));

  ui->comboBoxYoloImages->clear();
  ui->comboBoxYoloImages->addItem(
      tr("Do not touch the images"),
      static_cast<int>(ExportOptions::ImageMode::None));
  ui->comboBoxYoloImages->addItem(
      tr("Copy into the dataset"),
      static_cast<int>(ExportOptions::ImageMode::Copy));
  ui->comboBoxYoloImages->addItem(
      tr("Symlink into the dataset"),
      static_cast<int>(ExportOptions::ImageMode::Symlink));

  ui->comboBoxYoloClassOrder->clear();
  ui->comboBoxYoloClassOrder->addItem(
      tr("Alphabetical"),
      static_cast<int>(ExportOptions::ClassOrder::Alphabetical));
  ui->comboBoxYoloClassOrder->addItem(
      tr("First seen in the project"),
      static_cast<int>(ExportOptions::ClassOrder::FirstSeen));
}

void ExportDialog::populateFormatPage() {
  QWidget *page = ui->pageNative;
  QString hint;
  switch (m_format) {
    case ExportOptions::Format::Native:
      page = ui->pageNative;
      hint = tr("A single JSON file holding every annotation of the project, "
                "in the tool's own format.");
      break;
    case ExportOptions::Format::Coco:
      page = ui->pageCoco;
      hint = tr("A single COCO JSON file. Boxes become bbox entries and "
                "polygons become segmentation entries.");
      break;
    case ExportOptions::Format::Yolo:
      page = ui->pageYolo;
      hint = tr("A folder holding one .txt per image plus classes.txt. "
                "Polygons are written as segmentation rows.");
      break;
  }
  ui->stackedWidgetFormat->setCurrentWidget(page);
  ui->labelFormatHint->setText(hint);
}

void ExportDialog::scanProject() {
  m_typeCounts.clear();
  m_labelCounts.clear();
  if (!m_mgr) return;

  const QStringList ids = m_mgr->imageIds();
  QProgressDialog progress(tr("Scanning the project..."), tr("Cancel"), 0, 100,
                           this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(500);
  ProgressValue pVal(qMax(1, static_cast<int>(ids.size())));

  for (const QString &id : ids) {
    if (progress.wasCanceled()) break;
    bool cached;
    const Annotations ann = m_mgr->annotations(id, &cached);

    m_typeCounts[Helper::kBBox] += ann.bboxes.size();
    m_typeCounts[Helper::kPolygon] += ann.polygons.size();
    m_typeCounts[Helper::kLineStrip] += ann.line_strips.size();
    m_typeCounts[Helper::kCircle] += ann.circles.size();
    m_typeCounts[Helper::kPoint] += ann.points.size();
    m_typeCounts[Helper::kLine] += ann.lines.size();

    for (const auto &a : ann.bboxes) ++m_labelCounts[a.getLabel()];
    for (const auto &a : ann.polygons) ++m_labelCounts[a.getLabel()];
    for (const auto &a : ann.line_strips) ++m_labelCounts[a.getLabel()];
    for (const auto &a : ann.circles) ++m_labelCounts[a.getLabel()];
    for (const auto &a : ann.points) ++m_labelCounts[a.getLabel()];
    for (const auto &a : ann.lines) ++m_labelCounts[a.getLabel()];

    const int pv = pVal.value();
    if (pv > 0) progress.setValue(pv);
  }
  progress.setValue(100);

  for (auto type : ExportOptions::allTypes()) {
    QLabel *lb = countLabelFor(type);
    QComboBox *combo = comboFor(type);
    const int n = m_typeCounts.value(type, 0);
    if (lb) lb->setText(QString::number(n));
    // Nothing of this type in the project: keep the row visible for context
    // but do not let it suggest there is something to configure.
    if (combo) combo->setEnabled(n > 0);
  }
}

void ExportDialog::fillLabelTable() {
  ui->tableWidgetLabels->setRowCount(0);
  ui->tableWidgetLabels->setRowCount(m_labelCounts.size());

  int row = 0;
  for (auto it = m_labelCounts.cbegin(); it != m_labelCounts.cend(); ++it) {
    auto *labelItem = new QTableWidgetItem(it.key());
    labelItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable |
                        Qt::ItemIsUserCheckable);
    labelItem->setCheckState(Qt::Checked);
    ui->tableWidgetLabels->setItem(row, kLabelColumn, labelItem);

    auto *countItem = new QTableWidgetItem(QString::number(it.value()));
    countItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    countItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tableWidgetLabels->setItem(row, kCountColumn, countItem);

    auto *classItem = new QTableWidgetItem(it.key());
    classItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable |
                        Qt::ItemIsEditable);
    ui->tableWidgetLabels->setItem(row, kClassColumn, classItem);
    ++row;
  }
  ui->tableWidgetLabels->resizeColumnsToContents();
}

void ExportDialog::applyOptions(const ExportOptions &opt) {
  for (auto type : ExportOptions::allTypes()) {
    QComboBox *combo = comboFor(type);
    if (!combo) continue;
    const int idx =
        combo->findData(static_cast<int>(opt.handlingFor(type)));
    combo->setCurrentIndex(idx >= 0 ? idx : 0);
  }
  ui->spinBoxCircleSegments->setValue(opt.circleSegments);
  ui->doubleSpinBoxPointSize->setValue(opt.pointBoxSize);

  ui->checkBoxFilterLabels->setChecked(opt.filterLabels);
  for (int row = 0; row < ui->tableWidgetLabels->rowCount(); ++row) {
    QTableWidgetItem *labelItem = ui->tableWidgetLabels->item(row, kLabelColumn);
    QTableWidgetItem *classItem = ui->tableWidgetLabels->item(row, kClassColumn);
    if (!labelItem || !classItem) continue;
    const QString label = labelItem->text();
    if (opt.filterLabels) {
      labelItem->setCheckState(opt.includedLabels.contains(label)
                                   ? Qt::Checked
                                   : Qt::Unchecked);
    }
    const QString mapped = opt.labelMap.value(label);
    classItem->setText(mapped.isEmpty() ? label : mapped);
  }

  ui->doubleSpinBoxMinWidth->setValue(opt.minWidth);
  ui->doubleSpinBoxMinHeight->setValue(opt.minHeight);
  ui->doubleSpinBoxMinArea->setValue(opt.minArea);
  ui->checkBoxClip->setChecked(opt.clipToImage);
  ui->checkBoxDropOutside->setChecked(opt.dropOutside);
  ui->checkBoxSkipOccluded->setChecked(opt.skipOccluded);
  ui->checkBoxSkipTruncated->setChecked(opt.skipTruncated);
  ui->checkBoxSkipCrowded->setChecked(opt.skipCrowded);
  ui->checkBoxSkipEmptyImages->setChecked(opt.skipEmptyImages);

  int idx = ui->comboBoxYoloLayout->findData(static_cast<int>(opt.yoloLayout));
  ui->comboBoxYoloLayout->setCurrentIndex(idx >= 0 ? idx : 0);
  idx = ui->comboBoxYoloImages->findData(static_cast<int>(opt.imageMode));
  ui->comboBoxYoloImages->setCurrentIndex(idx >= 0 ? idx : 0);
  idx = ui->comboBoxYoloClassOrder->findData(static_cast<int>(opt.classOrder));
  ui->comboBoxYoloClassOrder->setCurrentIndex(idx >= 0 ? idx : 0);
  ui->checkBoxYoloDataYaml->setChecked(opt.writeDataYaml);
  ui->groupBoxYoloSplit->setChecked(opt.splitTrainVal);
  ui->doubleSpinBoxValRatio->setValue(opt.valRatio);
  ui->spinBoxSplitSeed->setValue(opt.splitSeed);

  ui->checkBoxNativePretty->setChecked(opt.prettyJson);
  ui->checkBoxCocoPretty->setChecked(opt.prettyJson);
  ui->checkBoxCocoEmptyImages->setChecked(opt.cocoIncludeEmptyImages);
  ui->checkBoxCocoIsCrowd->setChecked(opt.cocoIscrowdFromFlag);
}

ExportOptions ExportDialog::options() const {
  ExportOptions opt;
  opt.format = m_format;
  opt.outputPath = ui->lineEditOutput->text().trimmed();

  for (auto type : ExportOptions::allTypes()) {
    QComboBox *combo = comboFor(type);
    if (!combo) continue;
    opt.handling[type] = static_cast<ExportOptions::Handling>(
        combo->currentData().toInt());
  }
  opt.circleSegments = ui->spinBoxCircleSegments->value();
  opt.pointBoxSize = ui->doubleSpinBoxPointSize->value();

  opt.filterLabels = ui->checkBoxFilterLabels->isChecked();
  for (int row = 0; row < ui->tableWidgetLabels->rowCount(); ++row) {
    QTableWidgetItem *labelItem = ui->tableWidgetLabels->item(row, kLabelColumn);
    QTableWidgetItem *classItem = ui->tableWidgetLabels->item(row, kClassColumn);
    if (!labelItem) continue;
    const QString label = labelItem->text();
    if (labelItem->checkState() == Qt::Checked) {
      opt.includedLabels.insert(label);
    }
    if (classItem) {
      const QString mapped = classItem->text().trimmed();
      if (!mapped.isEmpty() && mapped != label) {
        opt.labelMap.insert(label, mapped);
      }
    }
  }

  opt.minWidth = ui->doubleSpinBoxMinWidth->value();
  opt.minHeight = ui->doubleSpinBoxMinHeight->value();
  opt.minArea = ui->doubleSpinBoxMinArea->value();
  opt.clipToImage = ui->checkBoxClip->isChecked();
  opt.dropOutside = ui->checkBoxDropOutside->isChecked();
  opt.skipOccluded = ui->checkBoxSkipOccluded->isChecked();
  opt.skipTruncated = ui->checkBoxSkipTruncated->isChecked();
  opt.skipCrowded = ui->checkBoxSkipCrowded->isChecked();
  opt.skipEmptyImages = ui->checkBoxSkipEmptyImages->isChecked();

  opt.yoloLayout = static_cast<ExportOptions::YoloLayout>(
      ui->comboBoxYoloLayout->currentData().toInt());
  opt.imageMode = static_cast<ExportOptions::ImageMode>(
      ui->comboBoxYoloImages->currentData().toInt());
  opt.classOrder = static_cast<ExportOptions::ClassOrder>(
      ui->comboBoxYoloClassOrder->currentData().toInt());
  opt.writeDataYaml = ui->checkBoxYoloDataYaml->isChecked();
  opt.splitTrainVal = ui->groupBoxYoloSplit->isChecked();
  opt.valRatio = ui->doubleSpinBoxValRatio->value();
  opt.splitSeed = ui->spinBoxSplitSeed->value();

  opt.prettyJson = (m_format == ExportOptions::Format::Coco)
                       ? ui->checkBoxCocoPretty->isChecked()
                       : ui->checkBoxNativePretty->isChecked();
  opt.cocoIncludeEmptyImages = ui->checkBoxCocoEmptyImages->isChecked();
  opt.cocoIscrowdFromFlag = ui->checkBoxCocoIsCrowd->isChecked();
  return opt;
}

void ExportDialog::updateOkState() {
  QPushButton *ok = ui->buttonBox->button(QDialogButtonBox::Ok);
  if (ok) ok->setEnabled(!ui->lineEditOutput->text().trimmed().isEmpty());
}

void ExportDialog::setAllChecked(bool checked) {
  for (int row = 0; row < ui->tableWidgetLabels->rowCount(); ++row) {
    if (ui->tableWidgetLabels->isRowHidden(row)) continue;
    if (QTableWidgetItem *item =
            ui->tableWidgetLabels->item(row, kLabelColumn)) {
      item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
  }
}

// ---------------------------------------------------------------- slots ----
void ExportDialog::on_toolButtonBrowse_clicked() {
  const QString current = ui->lineEditOutput->text();
  QString picked;
  if (m_format == ExportOptions::Format::Yolo) {
    picked = QFileDialog::getExistingDirectory(
        this, tr("Folder for the YOLO dataset"), current);
  } else {
    const QString caption = (m_format == ExportOptions::Format::Coco)
                                ? tr("COCO JSON file")
                                : tr("Native JSON file");
    picked = QFileDialog::getSaveFileName(this, caption, current,
                                          tr("JSON files (*.json)"));
    if (!picked.isEmpty() && QFileInfo(picked).suffix().isEmpty()) {
      picked += QStringLiteral(".json");
    }
  }
  if (!picked.isEmpty()) ui->lineEditOutput->setText(picked);
}

void ExportDialog::on_lineEditOutput_textChanged(const QString &) {
  updateOkState();
}

void ExportDialog::on_comboBoxYoloLayout_currentIndexChanged(int) {
  // Without an images/ tree there is nothing to place images into and nothing
  // for data.yaml to point at.
  const bool full = ui->comboBoxYoloLayout->currentData().toInt() ==
                    static_cast<int>(ExportOptions::YoloLayout::FullDataset);
  ui->comboBoxYoloImages->setEnabled(full);
  ui->labelYoloImages->setEnabled(full);
  ui->checkBoxYoloDataYaml->setEnabled(full);
}

void ExportDialog::on_checkBoxFilterLabels_toggled(bool on) {
  // The class renaming column stays usable either way; only the check boxes
  // depend on the filter being active.
  ui->pushButtonCheckAll->setEnabled(on);
  ui->pushButtonCheckNone->setEnabled(on);
  for (int row = 0; row < ui->tableWidgetLabels->rowCount(); ++row) {
    QTableWidgetItem *item = ui->tableWidgetLabels->item(row, kLabelColumn);
    if (!item) continue;
    Qt::ItemFlags flags = item->flags();
    flags.setFlag(Qt::ItemIsUserCheckable, on);
    item->setFlags(flags);
  }
}

void ExportDialog::on_pushButtonCheckAll_clicked() { setAllChecked(true); }

void ExportDialog::on_pushButtonCheckNone_clicked() { setAllChecked(false); }

void ExportDialog::on_pushButtonResetNames_clicked() {
  for (int row = 0; row < ui->tableWidgetLabels->rowCount(); ++row) {
    QTableWidgetItem *labelItem = ui->tableWidgetLabels->item(row, kLabelColumn);
    QTableWidgetItem *classItem = ui->tableWidgetLabels->item(row, kClassColumn);
    if (labelItem && classItem) classItem->setText(labelItem->text());
  }
}

void ExportDialog::on_lineEditLabelSearch_textChanged(const QString &text) {
  const QString needle = text.trimmed();
  for (int row = 0; row < ui->tableWidgetLabels->rowCount(); ++row) {
    QTableWidgetItem *item = ui->tableWidgetLabels->item(row, kLabelColumn);
    const bool match =
        needle.isEmpty() ||
        (item && item->text().contains(needle, Qt::CaseInsensitive));
    ui->tableWidgetLabels->setRowHidden(row, !match);
  }
}

void ExportDialog::on_pushButtonPreview_clicked() {
  if (!m_mgr) return;

  const ExportOptions opt = options();
  const QStringList ids = m_mgr->imageIds();

  QProgressDialog progress(tr("Running the current settings..."), tr("Cancel"),
                           0, 100, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(500);
  ProgressValue pVal(qMax(1, static_cast<int>(ids.size())));

  ExportStats stats;
  for (const QString &id : ids) {
    if (progress.wasCanceled()) {
      ui->labelSummary->setText(tr("Preview cancelled."));
      return;
    }
    bool cached;
    const Annotations ann = m_mgr->annotations(id, &cached);
    int w = ann.img_w;
    int h = ann.img_h;
    if (w <= 1 || h <= 1) {
      const QSize size = m_mgr->imageSize(id);
      w = size.width();
      h = size.height();
    }
    applyExportOptions(ann, opt, w, h, &stats);

    const int pv = pVal.value();
    if (pv > 0) progress.setValue(pv);
  }
  progress.setValue(100);

  const int classes = stats.perClass.size();
  QString summary =
      tr("%1 annotations in %2 images, %3 classes. %4 dropped by the current "
         "settings.")
          .arg(stats.kept)
          .arg(stats.images)
          .arg(classes)
          .arg(stats.dropped);
  if (stats.emptyImages > 0) {
    summary += QLatin1Char(' ');
    summary += opt.skipEmptyImages
                   ? tr("%1 empty images will be skipped.")
                         .arg(stats.emptyImages)
                   : tr("%1 images end up empty.").arg(stats.emptyImages);
  }
  if (m_format == ExportOptions::Format::Yolo && stats.boxes > 0 &&
      stats.polygons > 0) {
    summary += QLatin1Char('\n');
    summary += tr("Warning: the result mixes %1 detection rows with %2 "
                  "segmentation rows. A YOLO dataset must be one or the "
                  "other; convert the polygons to boxes, or skip the boxes.")
                   .arg(stats.boxes)
                   .arg(stats.polygons);
  }
  ui->labelSummary->setText(summary);
}

void ExportDialog::on_buttonBox_clicked(QAbstractButton *button) {
  if (ui->buttonBox->standardButton(button) !=
      QDialogButtonBox::RestoreDefaults) {
    return;
  }
  ExportOptions defaults;
  defaults.format = m_format;
  const bool nativeFormat = (m_format == ExportOptions::Format::Native);
  defaults.handling[Helper::kBBox] = ExportOptions::Handling::Native;
  defaults.handling[Helper::kPolygon] = ExportOptions::Handling::Native;
  const auto other = nativeFormat ? ExportOptions::Handling::Native
                                  : ExportOptions::Handling::Skip;
  defaults.handling[Helper::kLineStrip] = other;
  defaults.handling[Helper::kCircle] = other;
  defaults.handling[Helper::kPoint] = other;
  defaults.handling[Helper::kLine] = other;

  applyOptions(defaults);
  on_pushButtonResetNames_clicked();
  setAllChecked(true);
  ui->labelSummary->clear();
}
