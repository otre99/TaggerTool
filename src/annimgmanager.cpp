#include "annimgmanager.h"
#include "utils.h"
#include <QImage>
#include <QPointF>
#include <QRectF>
#include <QStringList>
#include <QTextStream>

AnnImgManager::AnnImgManager() = default;

void AnnImgManager::reset(const QString &images_folder_path,
                          const QString &annotations_folder_path,
                          const QString &ann_format,
                          const QStringList &image_ids) {
  m_imagesDir.setPath(images_folder_path);
  m_annotationsDir_.setPath(annotations_folder_path);
  if (image_ids.empty())
    m_imageIdsList = m_imagesDir.entryList(QStringList() << "*.jpg"
                                                         << "*.png"
                                                         << "*.bmp"
                                                         << "*.jpeg",
                                           QDir::Files);
  else
    m_imageIdsList = image_ids;
  m_annFormat = ann_format;
}

QMap<QString, QList<QRectF>>
AnnImgManager::annotations(const QString &image_id) {
  QFileInfo info(image_id);
  const QString ann_file_path =
      m_annotationsDir_.absoluteFilePath(info.baseName() + m_annFormat);

  if (m_annFormat == ".csv")
    return Helper::readCSVSingleFile(ann_file_path);
  if (m_annFormat == ".xml")
    return Helper::readXMLingleFile(ann_file_path);
}

QString AnnImgManager::annFilePath(const QString &image_id) {
  QFileInfo info(image_id);
  return m_annotationsDir_.absoluteFilePath(info.baseName() + m_annFormat);
}

QString AnnImgManager::imgFilePath(const QString &img_id) {
  return m_imagesDir.absoluteFilePath(img_id);
}

void AnnImgManager::saveAnnotations(const QString &image_id,
                                    const QMap<QString, QList<QRectF>> &ann,
                                    const QSize &img_size) {
  const QString ann_file_path = annFilePath(image_id);
  QSize final_size;
  if (img_size.isEmpty()) {
    final_size = imageSize(image_id);
  } else {
    final_size = img_size;
  }

  if (m_annFormat == ".csv")
    Helper::saveCSVSingleFile(ann_file_path, ann, image_id, final_size);

  if (m_annFormat == ".xml")
    Helper::saveXMLSingleFile(ann_file_path, ann, image_id, final_size);
}

QSize AnnImgManager::imageSize(const QString &image_id) {
  QImageReader img_info(imgFilePath(image_id));
  return img_info.size();
}

QImage AnnImgManager::image(const QString &image_id) {
  const QString img_file_path = m_imagesDir.absoluteFilePath(image_id);
  return QImage(img_file_path);
}
