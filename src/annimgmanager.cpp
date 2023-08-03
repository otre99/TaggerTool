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
  obj["label"] = label;
  obj["x1"] = x1;
  obj["y1"] = y1;
  obj["x2"] = x2;
  obj["y2"] = y2;
  return obj;
}
void Line::fromJson(const QJsonObject &obj) {
  label = obj["label"].toString();
  x1 = obj["x1"].toDouble();
  y1 = obj["y1"].toDouble();
  x2 = obj["x2"].toDouble();
  y2 = obj["y2"].toDouble();
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
  obj["label"] = label;
  obj["x1"] = x1;
  obj["y1"] = y1;
  obj["x2"] = x2;
  obj["y2"] = y2;
  obj["occluded"] = occluded;
  obj["truncated"] = truncated;
  obj["crowded"] = crowded;
  return obj;
}
void BBox::fromJson(const QJsonObject &obj) {
  label = obj["label"].toString();
  x1 = obj["x1"].toDouble();
  y1 = obj["y1"].toDouble();
  x2 = obj["x2"].toDouble();
  y2 = obj["y2"].toDouble();
  occluded = obj["occluded"].toBool(false);
  truncated = obj["truncated"].toBool(false);
  crowded = obj["crowded"].toBool(false);
}

BBox::BBox(float x1, float y1, float x2, float y2, const QString &lb,
           bool occluded, bool truncated, bool crowded)
    : x1{x1}, y1{y1}, x2{x2}, y2{y2}, occluded{occluded}, truncated{truncated},
      crowded{crowded} {
  label = lb;
}
BBox::BBox(const QRectF &r, const QString &lb, bool occluded, bool truncated,
           bool crowded)
    : x1(r.left()), y1(r.top()), x2(r.right()),
      y2(r.bottom()), occluded{occluded}, truncated{truncated}, crowded{
                                                                    crowded} {
  label = lb;
}

QPointF BBox::pt1() const { return {x1, y1}; }

QPointF BBox::pt2() const { return {x2, y2}; }

bool BBox::getOccluded() const { return occluded; }
bool BBox::getTruncated() const { return truncated; }
bool BBox::getCrowded() const { return crowded; }


// Circle impl
Circle::Circle(const QPointF &center, qreal radius, const QString &lb)
    : center1{center}, radius1{radius} {
  label = lb;
}

QPointF Circle::center() const {
  return center1;
}

qreal Circle::radius() const {
  return radius1;
}

QJsonObject  Circle::serializeJson() const {
  QJsonObject obj;
  obj["label"] = label;
  obj["x"] = center1.x();
  obj["y"] = center1.y();
  obj["radius"] = radius1;
  return obj;
}

void Circle::fromJson(const QJsonObject &obj){
  label = obj["label"].toString();
  center1.setX(obj["x"].toDouble());
  center1.setY(obj["y"].toDouble());
  radius1 = obj["radius"].toDouble();
}

// Point impl
Point::Point(float x, float y, const QString &lb) : x{x}, y{y} { label = lb; }
Point::Point(const QPointF &pt, const QString &lb) : x(pt.x()), y(pt.y()) {
  label = lb;
}

QPointF Point::pt() const { return {x, y}; }

QJsonObject Point::serializeJson() const {
  QJsonObject obj;
  obj["label"] = label;
  obj["x"] = x;
  obj["y"] = y;
  return obj;
}
void Point::fromJson(const QJsonObject &obj) {
  label = obj["label"].toString();
  x = obj["x"].toDouble();
  y = obj["y"].toDouble();
}

// Polygon impl
QJsonObject Polygon::serializeJson() const {
  QJsonObject obj;
  obj["label"] = label;

  QJsonArray jsonXArray, jsonYArray;
  const int n = this->xArray.size();
  for (int i = 0; i < n; ++i) {
    jsonXArray.append(xArray[i]);
    jsonYArray.append(yArray[i]);
  }
  obj["x_coords"] = jsonXArray;
  obj["y_coords"] = jsonYArray;
  return obj;
}
void Polygon::fromJson(const QJsonObject &obj) {
  label = obj["label"].toString();

  QJsonArray jsonXArray = obj["x_coords"].toArray();
  QJsonArray jsonYArray = obj["y_coords"].toArray();
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
  root.insert("image_name", image_name);
  root.insert("image_w", img_w);
  root.insert("image_h", img_h);

  // lines
  serializeJsonArrayOfObjects(lines, "lines", root);
  // boxes
  serializeJsonArrayOfObjects(bboxes, "bboxes", root);
  // circles
  serializeJsonArrayOfObjects(circles, "circles", root);
  // points
  serializeJsonArrayOfObjects(points, "points", root);
  // polygons
  serializeJsonArrayOfObjects(polygons, "polygons", root);
  // line_strips
  serializeJsonArrayOfObjects(line_strips, "line_strips", root);

  // description
  root["label"] = label;

  // description
  root["description"] = description;

  // tags
  QJsonArray array_tags;
  for (auto &tag : tags) {
    array_tags.append(tag);
  }
  root["tags"] = array_tags;
  return root;
}

void Annotations::fromJson(const QJsonObject &root) {
  image_name = root["image_name"].toString();
  img_w = root["image_w"].toInt();
  img_h = root["image_h"].toInt();

  // lines
  fromArrayOfJsonObjects(root["lines"].toArray(), lines);
  // bboxes
  fromArrayOfJsonObjects(root["bboxes"].toArray(), bboxes);
  // circles
  fromArrayOfJsonObjects(root["circles"].toArray(), circles);
  // points
  fromArrayOfJsonObjects(root["points"].toArray(), points);
  // polygons
  fromArrayOfJsonObjects(root["polygons"].toArray(), polygons);
  // line_strips
  fromArrayOfJsonObjects(root["line_strips"].toArray(), line_strips);

  // image label
  this->label = root["label"].toString();

  // description
  this->description = root["description"].toString();

  // tags
  const QJsonArray tags = root["tags"].toArray();
  for (const auto &tag : tags) {
    this->tags.append(tag.toString());
  }
}

template <typename T>
void Annotations::serializeJsonArrayOfObjects(const QVector<T> &anns, const QString &name, QJsonObject &root) const {
  QJsonArray array;
  for (auto &&ann : anns){
    array.push_back(ann.serializeJson());
  }
  root[name] = array;
}

template<typename T>
void  Annotations::fromArrayOfJsonObjects(const QJsonArray &json_arr, QVector<T> &output){
  output.clear();
  for (auto &&obj : json_arr) {
    T tobj;
    tobj.fromJson(obj.toObject());
    output.push_back(tobj);
  }
}

AnnImgManager::AnnImgManager() = default;

void AnnImgManager::reset(const QString &images_folder_path,
                          const QString &annotations_folder_path,
                          const QStringList &image_ids) {
  m_imagesDir.setPath(images_folder_path);
  m_annotationsDir.setPath(annotations_folder_path);
  if (image_ids.empty()) {
    m_imageIdsList = m_imagesDir.entryList(Helper::kImgExts, QDir::Files);
  } else {
    m_imageIdsList = image_ids;
  }

  m_annCache.clear();
  m_annCache.setMaxCost(m_imageIdsList.size());
}

const Annotations &AnnImgManager::annotations(const QString &image_id) {
  QFileInfo info(image_id);
  const QString ann_file_path =
      m_annotationsDir.absoluteFilePath(info.completeBaseName() + ".json");

  if (!m_annCache.contains(image_id)) {
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

  // qDebug() << "SAVING TO DISK!!!!";
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
  // qDebug() << "LOADING TO DISK!!!!";
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
