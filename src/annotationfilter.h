#ifndef ANNOTATIONFILTER_H
#define ANNOTATIONFILTER_H
#include <QList>
#include <QMap>
#include <QRectF>
#include <QString>

class AnnotationFilter {
  QMap<QString, QString> m_filteredLabels;

public:
  AnnotationFilter();
  void setFilteredLabels(const QMap<QString, QString> &filtered_labels);
  QMap<QString, QList<QRectF>>
  filter(const QMap<QString, QList<QRectF>> &bboxes);
};

#endif // ANNOTATIONFILTER_H
