#ifndef DIALOGEXPORTER_H
#define DIALOGEXPORTER_H

#include <QDialog>
class HeavyTaskThread;
namespace Ui {
class DialogExporter;
}

class DialogExporter : public QDialog {
  Q_OBJECT
  static const char COLLECT_ANNS[];

public:
  explicit DialogExporter(QWidget *parent, HeavyTaskThread *heavyTaskThread);
  void closeEvent(QCloseEvent *event) override;
  ~DialogExporter();

public slots:
  void taskStarted();
  void updateProgress(int p);
  void taskFinished(bool, QString errMsg);

private slots:
  void on_pBExportCoco_clicked();
  void on_pBExportPascal_clicked();

private:
  Ui::DialogExporter *ui;
  QString m_currProgressBarText;
  HeavyTaskThread *m_heavyTaskThread{nullptr};
  void getSelectedLabels();
};

#endif // DIALOGEXPORTER_H
