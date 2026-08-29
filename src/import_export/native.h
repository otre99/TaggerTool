#ifndef NATIVE_H
#define NATIVE_H
#include <QProgressDialog>

#include "annimgmanager.h"
#include "export_options.h"

bool exportProjectToJSON(AnnImgManager& mgr, const ExportOptions& opt,
                         QProgressDialog& progressDialog);

#endif  // NATIVE_H
