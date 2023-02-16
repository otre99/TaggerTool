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
  ~DialogExporter();

public slots:
  void taskStarted();
  void updateProgress(int p);
  void taskFinished(bool);

private:
  Ui::DialogExporter *ui;
  QString m_currProgressBarText;
  HeavyTaskThread *m_heavyTaskThread{nullptr};
};

#endif // DIALOGEXPORTER_H
