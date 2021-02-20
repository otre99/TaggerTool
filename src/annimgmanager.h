#ifndef ANNIMGMANAGER_H
#define ANNIMGMANAGER_H
#include <QDir>
#include <QImageReader>

class AnnImgManager {
  QDir m_imagesDir;
  QDir m_annotationsDir_;
  QStringList m_imageIdsList;
  QString m_annFormat;

public:
  AnnImgManager();
  void reset(const QString &images_folder_path,
             const QString &annotations_folder_path,
             const QString &ann_format = ".csv",
             const QStringList &image_ids = QStringList());

  void saveAnnotations(const QString &image_id,
                       const QMap<QString, QList<QRectF>> &ann,
                       const QSize &img_size = QSize());
  QMap<QString, QList<QRectF>> annotations(const QString &image_id);
  QSize imageSize(const QString &image_id);
  QImage image(const QString &image_id);
  QString annFilePath(const QString &img_id);
  QString imgFilePath(const QString &img_id);
  QStringList imageIds() const { return m_imageIdsList; }
};

#endif // ANNIMGMANAGER_H
