#include "loadimganndialog.h"
#include "ui_loadimganndialog.h"
#include <QFileDialog>

LoadImgAnnDialog::LoadImgAnnDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadImgAnnDialog)
{
    ui->setupUi(this);
}

LoadImgAnnDialog::~LoadImgAnnDialog()
{
    delete ui;
}

void LoadImgAnnDialog::setImgAndAnnFolders(const QString &imgf, const QString &annf)
{
    ui->leInputImageFolder->setText(imgf);
    ui->leInputAnnFolder->setText(annf);
}

QString LoadImgAnnDialog::annFolder() const {
    return ui->leInputAnnFolder->text();
}
QString LoadImgAnnDialog::imgFolder() const {
    return ui->leInputImageFolder->text();
}

void LoadImgAnnDialog::on_tbLoadImageFolder_clicked()
{
    const QString img_folder = QFileDialog::getExistingDirectory(
        this, "Images folder", ui->leInputImageFolder->text());
    if (img_folder.isEmpty()) return;
    ui->leInputImageFolder->setText(img_folder);
    ui->leInputAnnFolder->setText(img_folder);
}


void LoadImgAnnDialog::on_tbLoadAnnFolder_clicked()
{
    const QString img_folder = QFileDialog::getExistingDirectory(
        this, "Annotations folder", ui->leInputImageFolder->text());
    if (img_folder.isEmpty()) return;
    ui->leInputAnnFolder->setText(img_folder);
}

