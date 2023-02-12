#ifndef DIALOGLABELS_H
#define DIALOGLABELS_H

#include <QDialog>

namespace Ui {
class DialogLabels;
}

class DialogLabels : public QDialog {
  Q_OBJECT

public:
  explicit DialogLabels(QWidget *parent = nullptr, const QString title = {});
  ~DialogLabels();
  QStringList getLabels() const;
  void setLabels(const QStringList &labels);
private slots:
  void on_tBAdd_clicked();
  void on_tBRemove_clicked();

private:
  Ui::DialogLabels *ui;
};

#endif // DIALOGLABELS_H
