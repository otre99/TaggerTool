#include "yolo.h"

#include <QTextStream>
#include <algorithm>
#include <numeric>
#include <random>

#include "utils.h"

namespace {

/// Collect the exported class names carried by @p ann.
void collectClasses(const Annotations &ann, QSet<QString> *seen,
                    QStringList *firstSeen) {
  const auto add = [&](const QString &name) {
    if (name.isEmpty()) return;
    if (!seen->contains(name)) {
      seen->insert(name);
      firstSeen->append(name);
    }
  };
  for (const auto &b : ann.bboxes) add(b.getLabel());
  for (const auto &p : ann.polygons) add(p.getLabel());
  for (const auto &l : ann.line_strips) add(l.getLabel());
}

/// Resolve the export options for one image, filling in the image size.
Annotations resolveImage(AnnImgManager &mgr, const QString &imgId,
                         const ExportOptions &opt, int *outW, int *outH) {
  bool _;
  const Annotations raw = mgr.annotations(imgId, &_);
  int W = raw.img_w;
  int H = raw.img_h;
  if (W <= 1 || H <= 1) {
    const QSize size = mgr.imageSize(imgId);
    W = size.width();
    H = size.height();
  }
  *outW = W;
  *outH = H;
  return applyExportOptions(raw, opt, W, H);
}

bool writeTextFile(const QString &path, const QString &content) {
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    qWarning() << "Failed to open for write:" << path << f.errorString();
    return false;
  }
  QTextStream ts(&f);
  ts << content;
  return true;
}

/// Put the image next to its label file, by copy or by symlink.
bool placeImage(AnnImgManager &mgr, const QString &imgId, const QString &dstDir,
                ExportOptions::ImageMode mode) {
  if (mode == ExportOptions::ImageMode::None) return true;

  const QString src = mgr.imgFilePath(imgId);
  const QString dst = QDir(dstDir).filePath(QFileInfo(imgId).fileName());
  if (QFile::exists(dst)) QFile::remove(dst);

  const bool ok = (mode == ExportOptions::ImageMode::Copy)
                      ? QFile::copy(src, dst)
                      : QFile::link(src, dst);
  if (!ok) qWarning() << "Failed to place image:" << src << "->" << dst;
  return ok;
}

}  // namespace

bool exportProjectToYOLO(AnnImgManager &mgr, const ExportOptions &opt,
                         QProgressDialog &progressDialog) {
  const QStringList imageIds = mgr.imageIds();
  if (imageIds.isEmpty()) return false;

  progressDialog.setRange(0, 100);
  const auto report = [&](int done, int total, int base, int span) {
    if (total <= 0) return;
    progressDialog.setValue(base + (span * done) / total);
  };

  // --- Pass 1: find out what will actually be written ----------------------
  // Class ids must describe the exported result, not the raw project, so the
  // options are resolved once up front. Only the surviving image ids are kept
  // in memory; the annotations are resolved again while writing.
  QSet<QString> seen;
  QStringList firstSeen;
  QStringList keptIds;
  keptIds.reserve(imageIds.size());

  int done = 0;
  for (const QString &imgId : imageIds) {
    if (progressDialog.wasCanceled()) return false;
    int W = 0, H = 0;
    const Annotations ann = resolveImage(mgr, imgId, opt, &W, &H);
    if (opt.skipEmptyImages && annotationCount(ann) == 0) {
      report(++done, imageIds.size(), 0, 30);
      continue;
    }
    collectClasses(ann, &seen, &firstSeen);
    keptIds.append(imgId);
    report(++done, imageIds.size(), 0, 30);
  }

  QStringList classes = firstSeen;
  if (opt.classOrder == ExportOptions::ClassOrder::Alphabetical) {
    std::sort(classes.begin(), classes.end());
  }
  QHash<QString, int> classIndex;
  classIndex.reserve(classes.size());
  for (int i = 0; i < classes.size(); ++i) classIndex.insert(classes[i], i);

  // --- Train / val assignment ---------------------------------------------
  QSet<int> valSet;
  if (opt.splitTrainVal && !keptIds.isEmpty()) {
    std::vector<int> order(keptIds.size());
    std::iota(order.begin(), order.end(), 0);
    std::mt19937 rng(static_cast<unsigned>(opt.splitSeed));
    std::shuffle(order.begin(), order.end(), rng);

    const int nVal = std::clamp(
        static_cast<int>(std::llround(keptIds.size() * opt.valRatio)), 1,
        static_cast<int>(keptIds.size()) - 1);
    for (int i = 0; i < nVal; ++i) valSet.insert(order[i]);
  }

  // --- Output layout -------------------------------------------------------
  const bool fullDataset =
      (opt.yoloLayout == ExportOptions::YoloLayout::FullDataset);
  const QDir outDir(opt.outputPath);

  const auto labelsDirFor = [&](bool isVal) {
    QString dir = outDir.filePath(QStringLiteral("labels"));
    if (opt.splitTrainVal) {
      dir = QDir(dir).filePath(isVal ? QStringLiteral("val")
                                     : QStringLiteral("train"));
    }
    return dir;
  };
  const auto imagesDirFor = [&](bool isVal) {
    QString dir = outDir.filePath(QStringLiteral("images"));
    if (opt.splitTrainVal) {
      dir = QDir(dir).filePath(isVal ? QStringLiteral("val")
                                     : QStringLiteral("train"));
    }
    return dir;
  };

  for (bool isVal : {false, true}) {
    if (isVal && !opt.splitTrainVal) break;
    if (!QDir().mkpath(labelsDirFor(isVal))) {
      qWarning() << "Cannot create" << labelsDirFor(isVal);
      return false;
    }
    if (fullDataset && !QDir().mkpath(imagesDirFor(isVal))) {
      qWarning() << "Cannot create" << imagesDirFor(isVal);
      return false;
    }
  }

  // --- classes.txt ---------------------------------------------------------
  if (!writeTextFile(outDir.filePath(QStringLiteral("classes.txt")),
                     classes.join('\n') + (classes.isEmpty() ? "" : "\n"))) {
    return false;
  }

  // --- Pass 2: one label file per image ------------------------------------
  done = 0;
  for (int i = 0; i < keptIds.size(); ++i) {
    if (progressDialog.wasCanceled()) return false;

    const QString &imgId = keptIds[i];
    const bool isVal = valSet.contains(i);

    int W = 0, H = 0;
    const Annotations ann = resolveImage(mgr, imgId, opt, &W, &H);
    if (W <= 0 || H <= 0) {
      qWarning() << "Skipping" << imgId << ": unknown image size";
      report(++done, keptIds.size(), 30, 70);
      continue;
    }

    const QString base = QFileInfo(imgId).completeBaseName();
    const QString outFile = QDir(labelsDirFor(isVal)).filePath(base + ".txt");

    QFile f(outFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
      qWarning() << "Failed to open for write:" << outFile << f.errorString();
      return false;
    }
    QTextStream ts(&f);
    ts.setRealNumberNotation(QTextStream::FixedNotation);
    ts.setRealNumberPrecision(6);  // plenty for normalized coords

    // Detection rows: "class xc yc w h", all normalized.
    for (const auto &b : ann.bboxes) {
      const int cid = classIndex.value(b.getLabel(), -1);
      if (cid < 0) continue;

      const QRectF r = QRectF(b.pt1(), b.pt2()).normalized();
      const double xc = std::clamp(r.center().x() / W, 0.0, 1.0);
      const double yc = std::clamp(r.center().y() / H, 0.0, 1.0);
      const double wn = std::clamp(r.width() / W, 0.0, 1.0);
      const double hn = std::clamp(r.height() / H, 0.0, 1.0);
      ts << cid << ' ' << xc << ' ' << yc << ' ' << wn << ' ' << hn << '\n';
    }

    // Segmentation rows: "class x1 y1 x2 y2 ...", all normalized.
    const auto writePolygons = [&](const QVector<Polygon> &polys) {
      for (const auto &p : polys) {
        const int cid = classIndex.value(p.getLabel(), -1);
        if (cid < 0) continue;

        const QPolygonF poly = p.getPolygon();
        if (poly.size() < 3) continue;

        ts << cid;
        for (const QPointF &pt : poly) {
          ts << ' ' << std::clamp(pt.x() / W, 0.0, 1.0) << ' '
             << std::clamp(pt.y() / H, 0.0, 1.0);
        }
        ts << '\n';
      }
    };
    writePolygons(ann.polygons);
    writePolygons(ann.line_strips);
    f.close();

    if (fullDataset) {
      placeImage(mgr, imgId, imagesDirFor(isVal), opt.imageMode);
    }
    report(++done, keptIds.size(), 30, 70);
  }

  // --- data.yaml -----------------------------------------------------------
  // Only meaningful when the images sit next to the labels; with a labels-only
  // export there is no images/ tree for Ultralytics to point at.
  if (opt.writeDataYaml && fullDataset) {
    QString yaml;
    QTextStream ts(&yaml);
    ts << "# Generated by CVTaggerTool\n";
    ts << "path: " << outDir.absolutePath() << '\n';
    if (opt.splitTrainVal) {
      ts << "train: images/train\n";
      ts << "val: images/val\n";
    } else {
      ts << "train: images\n";
      ts << "val: images\n";
    }
    ts << "names:\n";
    for (int i = 0; i < classes.size(); ++i) {
      ts << "  " << i << ": " << classes[i] << '\n';
    }
    ts.flush();
    if (!writeTextFile(outDir.filePath(QStringLiteral("data.yaml")), yaml)) {
      return false;
    }
  }

  progressDialog.setValue(100);
  return true;
}
