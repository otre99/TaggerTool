#ifndef NATIVE_H
#define NATIVE_H
#include <QProgressDialog>

#include "annimgmanager.h"

bool exportProjectToJSON(AnnImgManager& mgr, const QString& outputFilePath,
                         QProgressDialog& progressDialog);

#endif  // NATIVE_H
