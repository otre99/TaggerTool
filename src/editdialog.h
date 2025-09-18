#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include <QDialog>

#include "utils.h"
namespace Ui {
class EditDialog;
}

class EditDialog : public QDialog {
  Q_OBJECT
 public:
  explicit EditDialog(Helper::CustomItemType anntype, const QString &label,
                      const QString &dsc, QWidget *parent = nullptr);
  ~EditDialog();
  QString label() const;
  QString description() const;
  void setLabel(const QString &lb);
  void setDescription(const QString &dsc);
  bool removeItem() const { return m_removedItem; }
  void setOccludedTrancatedCrowded(bool occluded, bool truncated, bool crowded);
  bool getOccluded() const;
  bool getTruncated() const;
  bool getCrowded() const;
  void accept() override;

 private slots:
  void on_toolButtonRemoveItem_clicked();

 private:
  Ui::EditDialog *ui;
  bool m_removedItem{false};
  QString m_oldLabel{};
  Helper::CustomItemType m_annType{};
};

#endif  // EDITDIALOG_H
