#include "loadimganndialog.h"

#include <QFileDialog>

#include "ui_loadimganndialog.h"
LoadImgAnnDialog::LoadImgAnnDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LoadImgAnnDialog) {
  ui->setupUi(this);
}

LoadImgAnnDialog::~LoadImgAnnDialog() { delete ui; }

void LoadImgAnnDialog::setImgAndAnnFolders(
    const QString &imgf, const QString &annf,
    const QList<RecentProjects::RecentEntry> &recents) {
  for (auto &&r : recents) {
    m_recents.emplace_back(r.imgFolder, r.annFolder);
  }

  QString imageChar =
      QString::fromUtf8(u8"\U0001F5BC");  // 🖼 (frame with picture)
  QString annoChar = QString::fromUtf8(u8"\U0001F4DD");  //  📝 (memo)

  ui->comboBoxRecents->addItem("");
  for (auto &&[imagePath, annotationPath, _] : recents) {
    QString msg = QString("%1: %2 - %3: %4")
                      .arg(imageChar, imagePath, annoChar, annotationPath);
    ui->comboBoxRecents->addItem(msg);
  }

  ui->leInputImageFolder->setText(imgf);
  ui->leInputAnnFolder->setText(annf);
}

QString LoadImgAnnDialog::annFolder() const {
  return ui->leInputAnnFolder->text();
}
QString LoadImgAnnDialog::imgFolder() const {
  return ui->leInputImageFolder->text();
}

void LoadImgAnnDialog::on_tbLoadImageFolder_clicked() {
  const QString img_folder = QFileDialog::getExistingDirectory(
      this, "Images folder", GetParentDir(ui->leInputImageFolder->text()));
  if (img_folder.isEmpty()) return;
  ui->leInputImageFolder->setText(img_folder);
  ui->leInputAnnFolder->setText(img_folder);
}

void LoadImgAnnDialog::on_tbLoadAnnFolder_clicked() {
  const QString img_folder = QFileDialog::getExistingDirectory(
      this, "Annotations folder", GetParentDir(ui->leInputImageFolder->text()));
  if (img_folder.isEmpty()) return;
  ui->leInputAnnFolder->setText(img_folder);
}

QString LoadImgAnnDialog::GetParentDir(const QString &dir_path) const {
  if (dir_path.isEmpty()) return dir_path;
  QDir dd(dir_path);
  dd.cdUp();
  return dd.absolutePath();
}

void LoadImgAnnDialog::on_comboBoxRecents_currentIndexChanged(int index) {
  if (index == 0) return;
  index -= 1;
  ui->leInputImageFolder->setText(m_recents[index].first);
  ui->leInputAnnFolder->setText(m_recents[index].second);
}
