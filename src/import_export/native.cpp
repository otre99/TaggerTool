#include "native.h"

#include <QJsonArray>

#include "utils.h"

bool exportProjectToJSON(AnnImgManager& mgr, const QString& outputFilePath,
                         QProgressDialog& progressDialog) {
  const QStringList imageIds = mgr.imageIds();
  QJsonArray root;
  ProgressValue pVal(imageIds.size());
  for (auto&& imgId : imageIds) {
    if (progressDialog.wasCanceled()) {
      return false;
    }
    bool _;
    auto&& ann = mgr.annotations(imgId, &_);
    root.append(ann.serializeJson());

    int pv;
    if ((pv = pVal.value()) > 0) {
      progressDialog.setValue(pv);
    }
  }

  // Write file
  QFile f(outputFilePath);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qWarning() << "Failed to open for write:" << outputFilePath
               << f.errorString();
    return false;
  }
  f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
  f.close();
  return true;
}
