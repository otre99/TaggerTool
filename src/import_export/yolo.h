#ifndef YOLO_H
#define YOLO_H
#include <QProgressDialog>

#include "annimgmanager.h"
#include "export_options.h"

bool exportProjectToYOLO(AnnImgManager& mgr, const ExportOptions& opt,
                         QProgressDialog& progressDialog);

#endif  // YOLO_H
