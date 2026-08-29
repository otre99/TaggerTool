#include "native.h"

#include <QJsonArray>
#include <QJsonDocument>

#include "utils.h"

bool exportProjectToJSON(AnnImgManager& mgr, const ExportOptions& opt,
                         QProgressDialog& progressDialog) {
  const QStringList imageIds = mgr.imageIds();
  QJsonArray root;
  ProgressValue pVal(qMax(1, static_cast<int>(imageIds.size())));

  for (auto&& imgId : imageIds) {
    if (progressDialog.wasCanceled()) {
      return false;
    }
    bool _;
    const Annotations raw = mgr.annotations(imgId, &_);

    int W = raw.img_w;
    int H = raw.img_h;
    if (W <= 1 || H <= 1) {
      const QSize size = mgr.imageSize(imgId);
      W = size.width();
      H = size.height();
    }

    const Annotations ann = applyExportOptions(raw, opt, W, H);
    if (opt.skipEmptyImages && annotationCount(ann) == 0) {
      const int skipped = pVal.value();
      if (skipped > 0) progressDialog.setValue(skipped);
      continue;
    }
    root.append(ann.serializeJson());

    const int pv = pVal.value();
    if (pv > 0) progressDialog.setValue(pv);
  }

  // Write file
  QFile f(opt.outputPath);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qWarning() << "Failed to open for write:" << opt.outputPath
               << f.errorString();
    return false;
  }
  f.write(QJsonDocument(root).toJson(opt.prettyJson ? QJsonDocument::Indented
                                                    : QJsonDocument::Compact));
  f.close();
  return true;
}
