#include "annotationfilter.h"

using Anntation = QMap<QString, QList<QRectF>>;

AnnotationFilter::AnnotationFilter() = default;

void AnnotationFilter::SetFilteredLabels(const QMap<QString,QString>& filtered_labels)
{
    filtered_labels_ = filtered_labels;
}

QMap<QString, QList<QRectF>> AnnotationFilter::Filter(const QMap<QString, QList<QRectF>> &bboxes)
{
    Anntation result;
    if (filtered_labels_.empty())
        return bboxes;

    const QStringList keys = filtered_labels_.keys();
    for (const auto &lb : keys) {
        if ( bboxes.contains(lb) ){
            for (const auto &box : bboxes[lb] ){
                result[ filtered_labels_[lb] ] << box;
            }
        }
    }
    return  result;
}
