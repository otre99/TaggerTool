#ifndef ANNIMGMANAGER_H
#define ANNIMGMANAGER_H
#include <QDir>
#include <QImageReader>

struct Line {
  QString label;
  float x1, y1, x2, y2;
};

struct Point {
  QString label;
  float x, y;
};

struct BBox {
  QString label;
  float x1, y1, x2, y2;
};

struct Polygon {
  QString label;
  QVector<float> xArray{};
  QVector<float> yArray{};
};

struct Annotations {
  int img_w{-1}, img_h{-1};
  QString image_name{};
  QVector<Line> lines{};
  QVector<Point> points{};
  QVector<BBox> bboxes{};
  QVector<Polygon> polygons{};
  QString label;
  QString description{};
  QStringList tags{};
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
};

#endif  // ANNIMGMANAGER_H
