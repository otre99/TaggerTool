#ifndef HEAVYTASKTHREAD_H
#define HEAVYTASKTHREAD_H

#include <QMutex>
#include <QObject>
#include <QProgressDialog>
#include <QThread>
#include <QVector>
#include <QWaitCondition>

#include "annimgmanager.h"

bool exportCOCOAnnotationsTask(AnnImgManager &mgr,
                               const QString &outputFilePath,
                               QProgressDialog &progressDialog,
                               bool includeBBoxes = true,
                               bool includePolygons = true);

#endif  // HEAVYTASKTHREAD_H
