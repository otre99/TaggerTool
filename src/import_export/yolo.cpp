#include "yolo.h"

#include "utils.h"

bool exportProjectToYOLO(AnnImgManager& mgr, const QString& outDir,
                         QProgressDialog& progressDialog, bool includeBBoxes,
                         bool includePolygons) {
  // 1) Scan once to collect all labels to build classes.txt
  QSet<QString> uniqueClasses;
  const auto imageIds = mgr.imageIds();
  for (const QString& imgId : imageIds) {
    bool _;
    const Annotations& ann = mgr.annotations(imgId, &_);
    if (includeBBoxes) {
      for (const auto& b : ann.bboxes) uniqueClasses.insert(b.getLabel());
    }
    if (includePolygons) {
      for (const auto& p : ann.polygons) uniqueClasses.insert(p.getLabel());
    }
  }

  QList<QString> classes(uniqueClasses.begin(), uniqueClasses.end());
  std::sort(classes.begin(), classes.end());
  QHash<QString, int> classIndex;
  classIndex.reserve(classes.size());
  for (int i = 0; i < classes.size(); ++i) classIndex[classes[i]] = i;

  // 2) Prepare output dirs
  const QString labelsDir = QDir(outDir).filePath("labels");
  QDir().mkpath(labelsDir);

  // 3) Write classes.txt (Ultralytics reads this to map id -> name)
  {
    QFile f(QDir(outDir).filePath("classes.txt"));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
      // handle error
      return false;
    }
    QTextStream ts(&f);
    for (auto&& cls : classes) {
      ts << cls << '\n';
    }
    f.close();
  }

  ProgressValue pVal(imageIds.size());

  // 4) Write one label file per image
  for (const QString& imgId : imageIds) {
    if (progressDialog.wasCanceled()) {
      return false;
    }

    bool _;
    const Annotations& ann = mgr.annotations(imgId, &_);

    int W = ann.img_w;
    int H = ann.img_h;
    if (W <= 1 || H <= 1) {
      auto imageReader = mgr.imageReader(imgId);
      W = imageReader.size().width();
      H = imageReader.size().height();
    }

    const QString base = QFileInfo(imgId).completeBaseName();
    const QString outFile = QDir(labelsDir).filePath(base + ".txt");
    QFile f(outFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
      // handle error
      return false;
    }
    QTextStream ts(&f);
    ts.setRealNumberNotation(QTextStream::FixedNotation);
    ts.setRealNumberPrecision(6);  // plenty for normalized coords

    // DET: one line per bbox
    if (includeBBoxes) {
      for (const auto& b : ann.bboxes) {
        const QString cls = b.getLabel();
        if (cls.isEmpty()) continue;

        const int cid = classIndex.value(cls, -1);
        if (cid < 0) continue;

        QPointF p1 = b.pt1();
        QPointF p2 = b.pt2();
        const double x = p1.x();
        const double y = p1.y();
        const double w = p2.x() - x;
        const double h = p2.y() - y;

        // centers
        const double xc = std::clamp((x + w * 0.5) / W, 0.0, 1.0);
        const double yc = std::clamp((y + h * 0.5) / H, 0.0, 1.0);
        const double wn = std::clamp(w / W, 0.0, 1.0);
        const double hn = std::clamp(h / H, 0.0, 1.0);

        // Ultralytics: "class xc yc w h"
        ts << cid << ' ' << xc << ' ' << yc << ' ' << wn << ' ' << hn << '\n';
      }

      int pv;
      if ((pv = pVal.value()) > 0) {
        progressDialog.setValue(pv);
      }
    }

    // SEG: one line per polygon
    if (includePolygons) {
      for (const auto& p : ann.polygons) {
        const QString cls = p.getLabel().trimmed();
        if (cls.isEmpty()) continue;
        const int cid = classIndex.value(cls, -1);
        if (cid < 0) continue;

        ts << cid;
        for (auto&& pt : p.getPolygon()) {
          const double xc = std::clamp(pt.x() / W, 0.0, 1.0);
          const double yc = std::clamp(pt.x() / W, 0.0, 1.0);
          ts << ' ' << xc << yc;
        }
        ts << '\n';
      }
    }
    f.close();
  }

  return true;
}
