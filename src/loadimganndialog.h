#ifndef LOADIMGANNDIALOG_H
#define LOADIMGANNDIALOG_H

#include <QDialog>

namespace Ui {
class LoadImgAnnDialog;
}

class LoadImgAnnDialog : public QDialog {
  Q_OBJECT

 public:
  explicit LoadImgAnnDialog(QWidget *parent = nullptr);
  ~LoadImgAnnDialog();
  void setImgAndAnnFolders(const QString &imgf, const QString &annf);
  QString annFolder() const;
  QString imgFolder() const;

 private slots:
  void on_tbLoadImageFolder_clicked();
  void on_tbLoadAnnFolder_clicked();

 private:
  Ui::LoadImgAnnDialog *ui;
  QString GetParentDir(const QString &dir_path) const;
};

#endif  // LOADIMGANNDIALOG_H
