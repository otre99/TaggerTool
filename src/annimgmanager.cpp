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
}

Annotations AnnImgManager::annotations(const QString &image_id) {
  QFileInfo info(image_id);
  const QString ann_file_path =
      m_annotationsDir.absoluteFilePath(info.completeBaseName() + ".json");
  return _loadAnnotation(ann_file_path);
}

QString AnnImgManager::annFilePath(const QString &image_id) {
  QFileInfo info(image_id);
  return m_annotationsDir.absoluteFilePath(info.baseName() + ".json");
}

QString AnnImgManager::imgFilePath(const QString &img_id) {
  return m_imagesDir.absoluteFilePath(img_id);
}

void AnnImgManager::saveAnnotations(const QString &image_id,
                                    const Annotations &ann) {
  const QString ann_file_path = annFilePath(image_id);
  _saveAnnotations(ann_file_path, ann);
}

void AnnImgManager::_saveAnnotations(const QString &path,
                                     const Annotations &ann) {
  QJsonObject root;
  root.insert(QString("image_name"), ann.image_name);
  root.insert(QString("image_w"), ann.img_w);
  root.insert(QString("image_h"), ann.img_h);

  // lines
  QJsonArray array_lines;
  for (auto &line : ann.lines) {
    QJsonObject obj;
    obj[QString("label")] = line.label;
    obj[QString("x1")] = line.x1;
    obj[QString("y1")] = line.y1;
    obj[QString("x2")] = line.x2;
    obj[QString("y2")] = line.y2;
    array_lines.append(obj);
  }
  root[QString("lines")] = array_lines;

  // boxes
  QJsonArray array_boxes;
  for (auto &bbox : ann.bboxes) {
    QJsonObject obj;
    obj[QString("label")] = bbox.label;
    obj[QString("x1")] = bbox.x1;
    obj[QString("y1")] = bbox.y1;
    obj[QString("x2")] = bbox.x2;
    obj[QString("y2")] = bbox.y2;
    array_boxes.append(obj);
  }
  root[QString("bboxes")] = array_boxes;

  // points
  QJsonArray array_points;
  for (auto &pt : ann.points) {
    QJsonObject obj;
    obj[QString("label")] = pt.label;
    obj[QString("x")] = pt.x;
    obj[QString("y")] = pt.y;
    array_points.append(obj);
  }
  root[QString("points")] = array_points;

  // polygons
  QJsonArray array_polygons;
  for (auto &poly : ann.polygons) {
    QJsonObject obj;
    obj[QString("label")] = poly.label;

    QJsonArray xArray, yArray;
    const int n = poly.xArray.size();
    for (int i = 0; i < n; ++i) {
      xArray.append(poly.xArray[i]);
      yArray.append(poly.yArray[i]);
    }
    obj[QString("x_coords")] = xArray;
    obj[QString("y_coords")] = yArray;
    array_polygons.append(obj);
  }
  root[QString("polygons")] = array_polygons;

  // description
  root[QString("label")] = ann.label;

  // description
  root[QString("description")] = ann.description;

  // tags
  QJsonArray array_tags;
  for (auto &tag : ann.tags) {
    array_tags.append(tag);
  }
  root[QString("tags")] = array_tags;

  const QByteArray out = QJsonDocument(root).toJson();
  QFile ofile(path);
  if (!ofile.open(QFile::WriteOnly)) {
    qDebug() << "Failed writing annotations: " << path;
    return;
  }
  ofile.write(out);
}

Annotations AnnImgManager::_loadAnnotation(const QString &path) {
  Annotations ann;
  QFile ifile(path);
  if (!ifile.open(QFile::ReadOnly)) {
    qDebug() << "Failed reading annotations file: " << path;
    return ann;
  }
  const QByteArray json = ifile.readAll();

  QJsonDocument doc = QJsonDocument::fromJson(json);
  QJsonObject root = doc.object();

  ann.image_name = root[QString("image_name")].toString();
  ann.img_w = root[QString("image_w")].toInt();
  ann.img_h = root[QString("image_h")].toInt();

  // lines
  const QJsonArray lines = root[QString("lines")].toArray();
  for (const auto &obj : lines) {
    Line line;
    line.label = obj[QString("label")].toString();
    line.x1 = obj[QString("x1")].toDouble();
    line.y1 = obj[QString("y1")].toDouble();
    line.x2 = obj[QString("x2")].toDouble();
    line.y2 = obj[QString("y2")].toDouble();
    ann.lines.push_back(line);
  }

  // bboxes
  const QJsonArray bboxes = root[QString("bboxes")].toArray();
  for (const auto &obj : bboxes) {
    BBox bbox;
    bbox.label = obj[QString("label")].toString();
    bbox.x1 = obj[QString("x1")].toDouble();
    bbox.y1 = obj[QString("y1")].toDouble();
    bbox.x2 = obj[QString("x2")].toDouble();
    bbox.y2 = obj[QString("y2")].toDouble();
    ann.bboxes.push_back(bbox);
  }

  // points
  const QJsonArray points = root[QString("points")].toArray();
  for (const auto &obj : points) {
    Point pt;
    pt.label = obj[QString("label")].toString();
    pt.x = obj[QString("x")].toDouble();
    pt.y = obj[QString("y")].toDouble();
    ann.points.push_back(pt);
  }

  // polygons
  const QJsonArray polygons = root[QString("polygons")].toArray();
  for (const auto &obj : polygons) {
    Polygon poly;
    poly.label = obj[QString("label")].toString();

    QJsonArray xArray = obj[QString("x_coords")].toArray();
    QJsonArray yArray = obj[QString("y_coords")].toArray();
    const int n = xArray.count();
    for (int i = 0; i < n; ++i) {
      poly.xArray.push_back(xArray[i].toDouble());
      poly.yArray.push_back(yArray[i].toDouble());
    }
    ann.polygons.push_back(poly);
  }

  // image label
  ann.label = root[QString("label")].toString();

  // description
  ann.description = root[QString("description")].toString();

  // tags
  const QJsonArray tags = root[QString("tags")].toArray();
  for (const auto &tag : tags) {
    ann.tags.append(tag.toString());
  }
  return ann;
}

QSize AnnImgManager::imageSize(const QString &image_id) {
  QImageReader img_info(imgFilePath(image_id));
  return img_info.size();
}

QImage AnnImgManager::image(const QString &image_id) {
  const QString img_file_path = m_imagesDir.absoluteFilePath(image_id);
  return QImage(img_file_path);
}
