#ifndef ANNOTATIONFILTER_H
#define ANNOTATIONFILTER_H
#include <QList>
#include <QString>
#include <QRectF>
#include <QMap>


class AnnotationFilter
{
    QMap<QString,QString> filtered_labels_;

public:
    AnnotationFilter();
    void SetFilteredLabels(const QMap<QString,QString>& filtered_labels);
    QMap<QString, QList<QRectF>> Filter(const QMap<QString, QList<QRectF>> &bboxes);
};

#endif // ANNOTATIONFILTER_H
