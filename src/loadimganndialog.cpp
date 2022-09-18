#include "loadimganndialog.h"

#include <QFileDialog>

#include "ui_loadimganndialog.h"

LoadImgAnnDialog::LoadImgAnnDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LoadImgAnnDialog) {
  ui->setupUi(this);
}

LoadImgAnnDialog::~LoadImgAnnDialog() { delete ui; }

void LoadImgAnnDialog::setImgAndAnnFolders(const QString &imgf,
                                           const QString &annf) {
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

QString  LoadImgAnnDialog::GetParentDir(const QString &dir_path) const {
    if (dir_path.isEmpty()) return dir_path;
    QDir dd(dir_path);
    dd.cdUp();
    return dd.absolutePath();
}
