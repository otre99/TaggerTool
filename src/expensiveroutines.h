#ifndef EXPENSIVEROUTINES_H
#define EXPENSIVEROUTINES_H

#include <QObject>
#include <QThread>
#include <annimgmanager.h>
#include "annotationfilter.h"

class ExpensiveRoutines : public QObject
{
    Q_OBJECT
    QThread *thread_;

    double max_iterations_;
    int p100_;
    void UpateProgress(double p);
    void ResetProgress(int n);

public:
    ExpensiveRoutines();
    ~ExpensiveRoutines() override;

public slots:
    void export_annotations(AnnImgManager *info,
                            AnnotationFilter filter,
                            const QString &output_ann_path,
                            int type_fmt);


signals:
    void JobStarted(bool);
    void Progress(int);
    void JobFinished(bool);
    void JobAborted(bool);
};

#endif // EXPENSIVEROUTINES_H
