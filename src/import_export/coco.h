#ifndef COCO_H
#define COCO_H
#include <QProgressDialog>

#include "annimgmanager.h"

bool exportCOCOAnnotationsTask(AnnImgManager &mgr,
                               const QString &outputFilePath,
                               QProgressDialog &progressDialog,
                               bool includeBBoxes = true,
                               bool includePolygons = true);

#endif  // COCO_H
