#include "annimgmanager.h"

#include <QDebug>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointF>
#include <QRectF>
#include <QStringList>
#include <QTextStream>

#include "utils.h"

// Line impl
QJsonObject Line::serializeJson() const {
  QJsonObject obj;
  obj[QString("label")] = label;
  obj[QString("x1")] = x1;
  obj[QString("y1")] = y1;
  obj[QString("x2")] = x2;
  obj[QString("y2")] = y2;
  return obj;
}
void Line::fromJson(const QJsonObject &obj) {
  label = obj[QString("label")].toString();
  x1 = obj[QString("x1")].toDouble();
  y1 = obj[QString("y1")].toDouble();
  x2 = obj[QString("x2")].toDouble();
  y2 = obj[QString("y2")].toDouble();
}

Line::Line(float x1, float y1, float x2, float y2, const QString &lb)
    : x1{x1}, y1{y1}, x2{x2}, y2{y2} {
  label = lb;
}

Line::Line(const QPointF &pt1, const QPointF &pt2, const QString &lb)
    : x1(pt1.x()), y1(pt1.y()), x2(pt2.x()), y2(pt2.y()) {
  label = lb;
}

QPointF Line::pt1() const { return {x1, y1}; }
QPointF Line::pt2() const { return {x2, y2}; }

// BBox impl
QJsonObject BBox::serializeJson() const {
  QJsonObject obj;
  obj[QString("label")] = label;
  obj[QString("x1")] = x1;
  obj[QString("y1")] = y1;
  obj[QString("x2")] = x2;
  obj[QString("y2")] = y2;
  obj[QString("occluded")] = occluded;
  obj[QString("truncated")] = truncated;
  obj[QString("crowded")] = crowded;
  return obj;
}
void BBox::fromJson(const QJsonObject &obj) {
  label = obj[QString("label")].toString();
  x1 = obj[QString("x1")].toDouble();
  y1 = obj[QString("y1")].toDouble();
  x2 = obj[QString("x2")].toDouble();
  y2 = obj[QString("y2")].toDouble();
  occluded = obj[QString("occluded")].toBool(false);
  truncated = obj[QString("truncated")].toBool(false);
  crowded = obj[QString("crowded")].toBool(false);
}

BBox::BBox(float x1, float y1, float x2, float y2, const QString &lb,
           bool occluded, bool truncated, bool crowded)
    : x1{x1}, y1{y1}, x2{x2}, y2{y2}, occluded{occluded}, truncated{truncated}, crowded{crowded} {
  label = lb;
}
BBox::BBox(const QRectF &r, const QString &lb, bool occluded, bool truncated, bool crowded)
    : x1(r.left()), y1(r.top()), x2(r.right()),
      y2(r.bottom()), occluded{occluded}, truncated{truncated}, crowded{crowded} {
  label = lb;
}

QPointF BBox::pt1() const { return {x1, y1}; }

QPointF BBox::pt2() const { return {x2, y2}; }

bool BBox::getOccluded() const { return occluded; }
bool BBox::getTruncated() const { return truncated; }
bool BBox::getCrowded() const { return crowded; }

// Point impl
Point::Point(float x, float y, const QString &lb) : x{x}, y{y} { label = lb; }
Point::Point(const QPointF &pt, const QString &lb) : x(pt.x()), y(pt.y()) {
  label = lb;
}

QPointF Point::pt() const { return {x, y}; }

QJsonObject Point::serializeJson() const {
  QJsonObject obj;
  obj[QString("label")] = label;
  obj[QString("x")] = x;
  obj[QString("y")] = y;
  return obj;
}
void Point::fromJson(const QJsonObject &obj) {
  label = obj[QString("label")].toString();
  x = obj[QString("x")].toDouble();
  y = obj[QString("y")].toDouble();
}

// Polygon impl
QJsonObject Polygon::serializeJson() const {
  QJsonObject obj;
  obj[QString("label")] = label;

  QJsonArray jsonXArray, jsonYArray;
  const int n = this->xArray.size();
  for (int i = 0; i < n; ++i) {
    jsonXArray.append(xArray[i]);
    jsonYArray.append(yArray[i]);
  }
  obj[QString("x_coords")] = jsonXArray;
  obj[QString("y_coords")] = jsonYArray;
  return obj;
}
void Polygon::fromJson(const QJsonObject &obj) {
  label = obj[QString("label")].toString();

  QJsonArray jsonXArray = obj[QString("x_coords")].toArray();
  QJsonArray jsonYArray = obj[QString("y_coords")].toArray();
  const int n = jsonXArray.count();
  for (int i = 0; i < n; ++i) {
    this->xArray.push_back(jsonXArray[i].toDouble());
    this->yArray.push_back(jsonYArray[i].toDouble());
  }
}

Polygon::Polygon(const QVector<float> &xArr, const QVector<float> &yArr,
                 const QString &lb)
    : xArray(xArr), yArray(yArr) {
  label = lb;
}
Polygon::Polygon(const QPolygonF &poly, const QString &lb) {
  label = lb;
  for (const auto &p : poly) {
    xArray.push_back(p.x());
    yArray.push_back(p.y());
  }
}

QPolygonF Polygon::getPolygon() const {
  QPolygonF poly;
  for (size_t i = 0; i < xArray.size(); ++i) {
    poly.append({xArray[i], yArray[i]});
  }
  return poly;
}

// Annotation impl
QJsonObject Annotations::serializeJson() const {
  QJsonObject root;
  root.insert(QString("image_name"), image_name);
  root.insert(QString("image_w"), img_w);
  root.insert(QString("image_h"), img_h);

  // lines
  QJsonArray array_lines;
  for (const auto &line : lines)
    array_lines.append(line.serializeJson());
  root[QString("lines")] = array_lines;

  // boxes
  QJsonArray array_boxes;
  for (const auto &bbox : bboxes)
    array_boxes.append(bbox.serializeJson());
  root[QString("bboxes")] = array_boxes;

  // points
  QJsonArray array_points;
  for (const auto &pt : points)
    array_points.append(pt.serializeJson());
  root[QString("points")] = array_points;

  // polygons
  QJsonArray array_polygons;
  for (const auto &poly : polygons)
    array_polygons.append(poly.serializeJson());
  root[QString("polygons")] = array_polygons;

  // line_strips
  QJsonArray array_line_strips;
  for (auto &poly : line_strips)
    array_line_strips.append(poly.serializeJson());
  root[QString("line_strips")] = array_line_strips;

  // description
  root[QString("label")] = label;

  // description
  root[QString("description")] = description;

  // tags
  QJsonArray array_tags;
  for (auto &tag : tags) {
    array_tags.append(tag);
  }
  root[QString("tags")] = array_tags;
  return root;
}

void Annotations::fromJson(const QJsonObject &root) {
  image_name = root[QString("image_name")].toString();
  img_w = root[QString("image_w")].toInt();
  img_h = root[QString("image_h")].toInt();

  // lines
  const QJsonArray jsonLines = root[QString("lines")].toArray();
  for (const auto &obj : jsonLines) {
    Line line;
    line.fromJson(obj.toObject());
    this->lines.push_back(line);
  }

  // bboxes
  const QJsonArray jsonBBoxes = root[QString("bboxes")].toArray();
  for (const auto &obj : jsonBBoxes) {
    BBox bbox;
    bbox.fromJson(obj.toObject());
    this->bboxes.push_back(bbox);
  }

  // points
  const QJsonArray jsonPoints = root[QString("points")].toArray();
  for (const auto &obj : jsonPoints) {
    Point pt;
    pt.fromJson(obj.toObject());
    this->points.push_back(pt);
  }

  // polygons
  const QJsonArray jsonPoly = root[QString("polygons")].toArray();
  for (const auto &obj : jsonPoly) {
    Polygon poly;
    poly.fromJson(obj.toObject());
    this->polygons.push_back(poly);
  }

  // line_strips
  const QJsonArray line_strips = root[QString("line_strips")].toArray();
  for (const auto &obj : line_strips) {
    Polygon poly;
    poly.fromJson(obj.toObject());
    this->line_strips.push_back(poly);
  }

  // image label
  this->label = root[QString("label")].toString();

  // description
  this->description = root[QString("description")].toString();

  // tags
  const QJsonArray tags = root[QString("tags")].toArray();
  for (const auto &tag : tags) {
    this->tags.append(tag.toString());
  }
}

AnnImgManager::AnnImgManager() = default;

void AnnImgManager::reset(const QString &images_folder_path,
                          const QString &annotations_folder_path,
                          const QStringList &image_ids) {
  m_imagesDir.setPath(images_folder_path);
  m_annotationsDir.setPath(annotations_folder_path);
  if (image_ids.empty())
    m_imageIdsList = m_imagesDir.entryList(Helper::kImgExts, QDir::Files);
  else
    m_imageIdsList = image_ids;

  m_annCache.clear();
  m_annCache.setMaxCost(m_imageIdsList.size());
}

const Annotations &AnnImgManager::annotations(const QString &image_id) {
  QFileInfo info(image_id);
  const QString ann_file_path =
      m_annotationsDir.absoluteFilePath(info.completeBaseName() + ".json");

  if (!m_annCache.contains(image_id)){
    const Annotations ann = _loadAnnotation(ann_file_path);
    m_annCache.insert(image_id, new Annotations(ann));
  }
  return *m_annCache[image_id];
}

QString AnnImgManager::annFilePath(const QString &image_id) {
  QFileInfo info(image_id);
  return m_annotationsDir.absoluteFilePath(basename(info.filePath()) + ".json");
}

QString AnnImgManager::imgFilePath(const QString &img_id) {
  return m_imagesDir.absoluteFilePath(img_id);
}

void AnnImgManager::saveAnnotations(const QString &image_id,
                                    const Annotations &ann) {
  const QString ann_file_path = annFilePath(image_id);
  m_annCache.insert(image_id, new Annotations(ann));
  _saveAnnotations(ann_file_path, ann);
}

void AnnImgManager::_saveAnnotations(const QString &path,
                                     const Annotations &ann) {

  qDebug() << "SAVING TO DISK!!!!";
  const QJsonObject root = ann.serializeJson();
  const QByteArray out = QJsonDocument(root).toJson();
  QFile ofile(path);
  if (!ofile.open(QFile::WriteOnly)) {
    qDebug() << "Failed writing annotations: " << path;
    return;
  }
  ofile.write(out);
}

Annotations AnnImgManager::_loadAnnotation(const QString &path) {
  qDebug() << "LOADING TO DISK!!!!";
  Annotations ann;
  QFile ifile(path);
  if (!ifile.open(QFile::ReadOnly)) {
    qDebug() << "Failed reading annotations file: " << path;
    return ann;
  }
  const QByteArray json = ifile.readAll();

  QJsonDocument doc = QJsonDocument::fromJson(json);
  QJsonObject root = doc.object();
  ann.fromJson(root);
  return ann;
}

QString AnnImgManager::basename(const QString &filePath) const {
  int i = filePath.size() - 1;
  while (filePath[i] != QChar('.')) {
    --i;
  }
  return filePath.mid(0, i);
}

QSize AnnImgManager::imageSize(const QString &image_id) {
  QImageReader img_info(imgFilePath(image_id));
  return img_info.size();
}

QImage AnnImgManager::image(const QString &image_id) {
  const QString img_file_path = m_imagesDir.absoluteFilePath(image_id);
  return QImage(img_file_path);
}
