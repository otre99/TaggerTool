#ifndef HEAVYTASKTHREAD_H
#define HEAVYTASKTHREAD_H

#include "annimgmanager.h"
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QVector>
#include <QWaitCondition>
#include <functional>

class HeavyTaskThread : public QThread {
  Q_OBJECT
public:
  enum TaskName {
    CollectAnnotations,
    ExportCOCOAnnotations,
  } m_currentTask;

  HeavyTaskThread();
  ~HeavyTaskThread();
  void run() override;
  void startTask(TaskName taskname);
  bool isTaskRunning() const;
  void killTaskAndWait();
signals:
  void progress(int);
  void taskFinished(bool);
  void taskStarted();

private:
  size_t m_ncount;
  size_t m_index;
  QWaitCondition m_waitCond;
  QMutex m_mutex;
  bool m_exit_{false};
  bool m_abort_{false};
  bool m_taskIsRunning{false};
  void internalStartTask(std::function<bool()> funct);

  void updateProgress(size_t index);
  bool collectAllAnnotationsTask();
  bool exportCOCOAnnotationsTask();

public:
  // CollectAnnotations
  AnnImgManager *annImgManager{nullptr};
  QSet<QString> uniqueLabels;
  QString outputDirOrFile;
};

#endif // HEAVYTASKTHREAD_H
