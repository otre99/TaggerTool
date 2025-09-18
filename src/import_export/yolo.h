#ifndef YOLO_H
#define YOLO_H
#include <QProgressDialog>

#include "annimgmanager.h"

bool exportProjectToYOLO(AnnImgManager& mgr, const QString& outputDirPath,
                         QProgressDialog& progressDialog,
                         bool includeBBoxes = true,
                         bool includePolygons = true);

#endif  // YOLO_H
