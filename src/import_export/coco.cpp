#include "coco.h"

#include <QJsonArray>
#include <QJsonDocument>

#include "utils.h"

namespace {

/// Assigns stable, 1 based COCO ids to exported class names.
class CategoryIndex {
 public:
  int idFor(const QString &name) {
    if (name.isEmpty()) return -1;
    auto it = m_map.constFind(name);
    if (it != m_map.constEnd()) return it.value();
    const int id = m_next++;
    m_map.insert(name, id);
    m_order.append(name);
    return id;
  }

  QJsonArray toJson() const {
    QJsonArray arr;
    for (const QString &name : m_order) {
      arr.append(QJsonObject{{"id", m_map.value(name)},
                             {"name", name},
                             {"supercategory", QStringLiteral("object")}});
    }
    return arr;
  }

 private:
  QHash<QString, int> m_map;
  QStringList m_order;
  int m_next{1};
};

}  // namespace

bool exportCOCOAnnotationsTask(AnnImgManager &mgr, const ExportOptions &opt,
                               QProgressDialog &progressDialog) {
  int next_image_id = 1;
  int next_ann_id = 1;

  CategoryIndex categories;
  QJsonArray images;
  QJsonArray annotations;

  const QStringList imageIds = mgr.imageIds();
  ProgressValue pVal(qMax(1, static_cast<int>(imageIds.size())));

  for (const QString &imgId : imageIds) {
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
    const bool empty = (annotationCount(ann) == 0);
    if (empty && (opt.skipEmptyImages || !opt.cocoIncludeEmptyImages)) {
      const int skipped = pVal.value();
      if (skipped > 0) progressDialog.setValue(skipped);
      continue;
    }

    const int image_id = next_image_id++;
    images.append(QJsonObject{
        {"id", image_id}, {"file_name", imgId}, {"width", W}, {"height", H}});

    // ---- detections ------------------------------------------------------
    for (const auto &b : ann.bboxes) {
      const int cat_id = categories.idFor(b.getLabel());
      if (cat_id < 0) continue;

      const QRectF r = QRectF(b.pt1(), b.pt2()).normalized();
      annotations.append(QJsonObject{
          {"id", next_ann_id++},
          {"image_id", image_id},
          {"category_id", cat_id},
          {"iscrowd", (opt.cocoIscrowdFromFlag && b.getCrowded()) ? 1 : 0},
          {"bbox", QJsonArray{r.x(), r.y(), r.width(), r.height()}},
          {"area", r.width() * r.height()}});
    }

    // ---- segmentations ---------------------------------------------------
    for (const auto &p : ann.polygons) {
      const int cat_id = categories.idFor(p.getLabel());
      if (cat_id < 0) continue;

      const QPolygonF poly = p.getPolygon();
      if (poly.size() < 3) continue;

      QJsonArray flat;
      for (const QPointF &pt : poly) {
        flat.append(pt.x());
        flat.append(pt.y());
      }

      // COCO stores the enclosing box alongside the segmentation.
      const QRectF br = poly.boundingRect();
      annotations.append(
          QJsonObject{{"id", next_ann_id++},
                      {"image_id", image_id},
                      {"category_id", cat_id},
                      {"iscrowd", 0},
                      {"segmentation", QJsonArray{flat}},
                      {"area", Helper::polygonArea(poly)},
                      {"bbox", QJsonArray{br.x(), br.y(), br.width(),
                                          br.height()}}});
    }

    const int pv = pVal.value();
    if (pv > 0) progressDialog.setValue(pv);
  }

  QJsonObject root;
  root.insert(
      "info",
      QJsonObject{{"year", QDate::currentDate().year()},
                  {"version", "1.0"},
                  {"description", "Exported by CVTaggerTool"},
                  {"date_created",
                   QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}});
  root.insert("licenses", QJsonArray{});
  root.insert("images", images);
  root.insert("annotations", annotations);
  root.insert("categories", categories.toJson());

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
