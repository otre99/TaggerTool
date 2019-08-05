#include "annimgmanager.h"
#include <QStringList>
#include <QTextStream>
#include <QPointF>
#include <QRectF>
#include <QImage>
#include "utils.h"

AnnImgManager::AnnImgManager() = default;

void AnnImgManager::Reset(const QString &images_folder_path,
                          const QString &annotations_folder_path,
                          const QString &ann_format, const QStringList& image_ids)
{
    images_dir_.setPath(images_folder_path);
    annotations_dir_.setPath(annotations_folder_path);
    if ( image_ids.empty() )
        image_ids_list_ = images_dir_.entryList( QStringList() << "*.jpg"<< "*.png" << "*.bmp" << "*.jpeg", QDir::Files);
    else
        image_ids_list_ = image_ids;
    ann_format_ = ann_format;
}


QMap<QString, QList<QRectF> > AnnImgManager::GetAnnotations(const QString &image_id)
{
    QFileInfo info(image_id);
    const QString ann_file_path = annotations_dir_.absoluteFilePath(info.baseName() + ann_format_ );

    if (ann_format_ == ".csv" )
        return Helper::ReadCSVSingleFile(ann_file_path);
    if (ann_format_ == ".xml")
        return Helper::ReadXMLingleFile(ann_file_path);
}

QString AnnImgManager::GetAnnFilePath(const QString &image_id)
{
    QFileInfo info(image_id);
    return annotations_dir_.absoluteFilePath(info.baseName() + ann_format_ );
}

QString AnnImgManager::GetImgFilePath(const QString &img_id)
{
    return images_dir_.absoluteFilePath(img_id);
}

void AnnImgManager::SaveAnnotations(const QString &image_id, const QMap<QString, QList<QRectF> > &ann, const QSize &img_size)
{
    const QString ann_file_path = GetAnnFilePath(image_id);
    QSize final_size;
    if ( img_size.isEmpty() ){
        final_size = GetImageSizeFromImage(image_id);
    } else {
        final_size=img_size;
    }

    if ( ann_format_ == ".csv" )
        Helper::SaveCSVSingleFile(ann_file_path, ann, image_id, final_size);

    if ( ann_format_ == ".xml" )
        Helper::SaveXMLSingleFile(ann_file_path, ann, image_id, final_size);
}

QSize AnnImgManager::GetImageSizeFromImage(const QString &image_id)
{
    QImageReader img_info( GetImgFilePath(image_id) );
    return  img_info.size();
}



QImage AnnImgManager::GetImage(const QString &image_id)
{
    const QString img_file_path = images_dir_.absoluteFilePath(image_id);
    return QImage(img_file_path);
}
