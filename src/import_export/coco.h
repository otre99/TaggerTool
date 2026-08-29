#ifndef COCO_H
#define COCO_H
#include <QProgressDialog>

#include "annimgmanager.h"
#include "export_options.h"

bool exportCOCOAnnotationsTask(AnnImgManager &mgr, const ExportOptions &opt,
                               QProgressDialog &progressDialog);

#endif  // COCO_H
