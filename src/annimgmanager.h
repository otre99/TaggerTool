#ifndef ANNIMGMANAGER_H
#define ANNIMGMANAGER_H
#include <QDir>
#include <QImageReader>

class Annotation {
public:
  QString getLabel() const { return label; }
  void setLabel(QString &label) { this->label = label; }
  virtual QJsonObject serializeJson() const = 0;
  virtual void fromJson(const QJsonObject &obj) = 0;

protected:
  QString label;
};

class Line : public Annotation {
  float x1, y1, x2, y2;

public:
  Line() = default;
  Line(float x1, float y1, float x2, float y2, const QString &lb);
  Line(const QPointF &pt1, const QPointF &pt2, const QString &lb);
  QPointF pt1() const;
  QPointF pt2() const;
  virtual QJsonObject serializeJson() const override;
  virtual void fromJson(const QJsonObject &obj) override;
};

class BBox : public Annotation {
  float x1, y1, x2, y2;
  bool occluded{false};
  bool truncated{false};

public:
  BBox() = default;
  BBox(float x1, float y1, float x2, float y2, const QString &lb,
       bool occluded = false, bool truncated = false);
  BBox(const QRectF &r, const QString &lb, bool occluded = false,
       bool truncated = false);
  QPointF pt1() const;
  QPointF pt2() const;
  virtual QJsonObject serializeJson() const override;
  virtual void fromJson(const QJsonObject &obj) override;
};

class Point : public Annotation {
  float x, y;

public:
  Point() = default;
  Point(float x, float y, const QString &lb);
  Point(const QPointF &pt, const QString &lb);
  QPointF pt() const;
  virtual QJsonObject serializeJson() const override;
  virtual void fromJson(const QJsonObject &obj) override;
};

class Polygon : public Annotation {
  QVector<float> xArray{};
  QVector<float> yArray{};

public:
  Polygon() = default;
  Polygon(const QVector<float> &xArr, const QVector<float> &yArr,
          const QString &lb);
  Polygon(const QPolygonF &poly, const QString &lb);
  QPolygonF getPolygon() const;
  virtual QJsonObject serializeJson() const override;
  virtual void fromJson(const QJsonObject &obj) override;
};

struct Annotations {
  int img_w{-1}, img_h{-1};
  QString image_name{};
  QVector<Line> lines{};
  QVector<Point> points{};
  QVector<BBox> bboxes{};
  QVector<Polygon> polygons{};
  QVector<Polygon> line_strips{};
  QString label;
  QString description{};
  QStringList tags{};
  QJsonObject serializeJson() const;
  void fromJson(const QJsonObject &obj);
};

class AnnImgManager {
  QDir m_imagesDir;
  QDir m_annotationsDir;
  QStringList m_imageIdsList;

public:
  AnnImgManager();
  void reset(const QString &images_folder_path,
             const QString &annotations_folder_path,
             const QStringList &image_ids = QStringList());

  void saveAnnotations(const QString &image_id, const Annotations &ann);

  QString annFolder() const { return m_annotationsDir.absolutePath(); }
  QString imgFolder() const { return m_imagesDir.absolutePath(); }

  Annotations annotations(const QString &image_id);
  QSize imageSize(const QString &image_id);
  QImage image(const QString &image_id);
  QString annFilePath(const QString &img_id);
  QString imgFilePath(const QString &img_id);
  QStringList imageIds() const { return m_imageIdsList; }

private:
  void _saveAnnotations(const QString &path, const Annotations &ann);
  Annotations _loadAnnotation(const QString &path);
  QString basename(const QString &filePath) const;
};

#endif // ANNIMGMANAGER_H
