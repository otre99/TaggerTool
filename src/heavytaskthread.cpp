#include "heavytaskthread.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>

#include "utils.h"

bool exportCOCOAnnotationsTask(AnnImgManager& annImgManager,
                               const QString& outputFilePath,
                               QProgressDialog& progressDialog,
                               bool includeBBoxes, bool includePolygons) {
  struct CocoIds {
    // integer IDs required by COCO
    int next_image_id = 1;
    int next_ann_id = 1;
    int next_cat_id = 1;
  } ids;

  struct CocoCategoryIndex {
    // label -> category_id
    QHash<QString, int> map;
    int idForLabel(const QString& label, CocoIds& ids) {
      if (label.isEmpty()) return -1;
      auto it = map.constFind(label);
      if (it != map.constEnd()) return it.value();
      int id = ids.next_cat_id++;
      map.insert(label, id);
      return id;
    }
  } catIndex;

  QJsonArray images;
  QJsonArray annotations;
  QJsonArray categories;  // filled at the end from catIndex

  // We’ll collect categories first while we walk
  struct PendingCategory {
    int id;
    QString name;
    QString supercategory;
  };
  QHash<int, PendingCategory> catDefs;

  const QStringList imageIds = annImgManager.imageIds();

  // --- Build images + annotations
  for (const QString& imgId : imageIds) {
    bool _;
    const Annotations& ann = annImgManager.annotations(imgId, &_);

    // images[]
    const int image_id = ids.next_image_id++;
    QJsonObject imgObj{
        {"id", image_id},
        {"file_name", ann.image_name.isEmpty() ? imgId : ann.image_name},
        {"width", ann.img_w},
        {"height", ann.img_h}};
    images.append(imgObj);

    // annotations[] from bboxes
    if (includeBBoxes) {
      for (const auto& b : ann.bboxes) {
        const QString label = b.getLabel().trimmed();
        const int cat_id = catIndex.idForLabel(label, ids);
        if (cat_id > 0 && !catDefs.contains(cat_id)) {
          catDefs.insert(
              cat_id, PendingCategory{cat_id, label, QStringLiteral("object")});
        }

        const QPointF pt1 = b.pt1();
        const QPointF pt2 = b.pt2();

        // COCO bbox: [x, y, width, height], top-left origin
        const double w = std::abs(pt2.x() - pt1.x());
        const double h = std::abs(pt2.y() - pt1.y());
        const double ar = w * h;

        QJsonObject a{{"id", ids.next_ann_id++},
                      {"image_id", image_id},
                      {"category_id", cat_id > 0 ? cat_id : 0},
                      {"iscrowd", 0},
                      {"bbox", QJsonArray{pt1.x(), pt1.y(), w, h}},
                      {"area", ar}};
        annotations.append(a);
      }
    }

    // annotations[] from polygons (as segmentations)
    if (includePolygons) {
      for (const auto& p : ann.polygons) {
        const QString label = p.getLabel().trimmed();
        const int cat_id = catIndex.idForLabel(label, ids);
        if (cat_id > 0 && !catDefs.contains(cat_id)) {
          catDefs.insert(
              cat_id, PendingCategory{cat_id, label, QStringLiteral("object")});
        }

        // COCO expects segmentation as a list of polygons (each is a flat
        // [x1,y1,...]) Ensure lengths match and >= 3 points
        const QPolygonF poly = p.getPolygon();
        const int n = poly.size();
        if (n >= 3) {
          QJsonArray flat;
          for (auto&& coord : poly) {
            flat.append(coord.x());
            flat.append(coord.y());
          }

          // for (int i = 0; i < n; ++i) {
          //     flat.append()
          //   // flat.append(p.x_coords[i]);
          //   // flat.append(p.y_coords[i]);
          // }
          const double ar = Helper::polygonArea(poly);

          auto&& [l, r] = std::minmax_element(
              poly.begin(), poly.end(),
              [](auto& p1, auto& p2) { return p1.x() < p2.x(); });

          auto&& [t, b] = std::minmax_element(
              poly.begin(), poly.end(),
              [](auto& p1, auto& p2) { return p1.y() < p2.y(); });

          const double x = l->x();
          const double y = t->x();
          const double w = r->x() - x;
          const double h = b->x() - y;

          QJsonArray segm;
          segm.append(flat);
          QJsonObject a{{"id", ids.next_ann_id++},
                        {"image_id", image_id},
                        {"category_id", cat_id > 0 ? cat_id : 0},
                        {"iscrowd", 0},
                        {"segmentation", segm},
                        {"area", ar},
                        {"bbox", QJsonArray{x, y, w, h}}};

          annotations.append(a);
        }
      }
    }

    // --- TODO: If you want to export circles, approximate as polygon here
    //           (e.g., 32-gon) and push to annotations like polygons above.
  }

  // categories[] from collected catDefs
  for (auto it = catDefs.cbegin(); it != catDefs.cend(); ++it) {
    const auto& c = it.value();
    categories.append(QJsonObject{
        {"id", c.id}, {"name", c.name}, {"supercategory", c.supercategory}});
  }

  // root COCO object
  QJsonObject root;
  root.insert(
      "info",
      QJsonObject{{"year", QDate::currentDate().year()},
                  {"version", "1.0"},
                  {"description", "Exported by CVTaggerTool"},
                  {"date_created",
                   QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}});
  root.insert("licenses", QJsonArray{});  // optional
  root.insert("images", images);
  root.insert("annotations", annotations);
  root.insert("categories", categories);

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
