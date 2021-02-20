#ifndef EXPENSIVEROUTINES_H
#define EXPENSIVEROUTINES_H

#include "annotationfilter.h"
#include <QObject>
#include <QThread>
#include <annimgmanager.h>

class ExpensiveRoutines : public QObject {
  Q_OBJECT
  QThread *m_thread;
  double m_maxIterations;
  int m_p100;
  void upateProgress(double p);
  void resetProgress(int n);

public:
  ExpensiveRoutines();
  ~ExpensiveRoutines() override;

public slots:
  void exportAnnotations(AnnImgManager *info, AnnotationFilter filter,
                         const QString &output_ann_path, int type_fmt);
signals:
  void jobStarted(bool);
  void progress(int);
  void jobFinished(bool);
  void jobAborted(bool);
};

#endif // EXPENSIVEROUTINES_H
