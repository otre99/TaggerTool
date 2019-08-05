#ifndef ANNIMGMANAGER_H
#define ANNIMGMANAGER_H
#include <QDir>
#include <QImageReader>

class AnnImgManager
{
    QDir images_dir_;
    QDir annotations_dir_;
    QStringList image_ids_list_;
    QString ann_format_;


public:
    AnnImgManager();
    void Reset(const QString &images_folder_path,
               const QString &annotations_folder_path,
               const QString &ann_format=".csv",
               const QStringList& image_ids=QStringList());


    void SaveAnnotations(const QString &image_id, const QMap<QString, QList<QRectF> > &ann, const QSize &img_size=QSize());
    QMap<QString, QList<QRectF> > GetAnnotations(const QString &image_id);
    QSize GetImageSizeFromImage(const QString &image_id);
    QImage GetImage(const QString &image_id) ;
    QString GetAnnFilePath(const QString &img_id) ;
    QString GetImgFilePath(const QString &img_id) ;
    QStringList GetListOfImageIds() const {return  image_ids_list_; }
};

#endif // ANNIMGMANAGER_H
