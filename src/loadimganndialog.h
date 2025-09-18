#ifndef LOADIMGANNDIALOG_H
#define LOADIMGANNDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class LoadImgAnnDialog;
}

class LoadImgAnnDialog : public QDialog {
  Q_OBJECT

 public:
  explicit LoadImgAnnDialog(QWidget *parent = nullptr);
  ~LoadImgAnnDialog();
  void setImgAndAnnFolders(
      const QString &imgf, const QString &annf,
      const QList<std::tuple<QString, QString, qint64>> &recents);
  QString annFolder() const;
  QString imgFolder() const;

 private slots:
  void on_tbLoadImageFolder_clicked();
  void on_tbLoadAnnFolder_clicked();

  void on_comboBoxRecents_currentIndexChanged(int index);

 private:
  Ui::LoadImgAnnDialog *ui;
  QString GetParentDir(const QString &dir_path) const;
  QSettings m_settings;
  QList<QPair<QString, QString>> m_recents;
};

#endif  // LOADIMGANNDIALOG_H
