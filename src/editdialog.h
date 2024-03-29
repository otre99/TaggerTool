#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDialog>

namespace Ui {
class EditDialog;
}

class EditDialog : public QDialog {
  Q_OBJECT
public:
  explicit EditDialog(QWidget *parent = nullptr);
  ~EditDialog();
  QString label() const;
  void setLabel(const QString &lb);
  bool removeItem() const { return m_removedItem; }
  void setOccludedTrancatedCrowded(bool occluded, bool truncated, bool crowded);
  bool getOccluded() const;
  bool getTruncated() const;
  bool getCrowded() const;
private slots:
  void on_toolButtonRemoveItem_clicked();

private:
  Ui::EditDialog *ui;
  bool m_removedItem{false};
};

#endif // EDITDIALOG_H
