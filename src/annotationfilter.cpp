#include "annotationfilter.h"

using Anntation = QMap<QString, QList<QRectF>>;

AnnotationFilter::AnnotationFilter() = default;

void AnnotationFilter::setFilteredLabels(
    const QMap<QString, QString> &filtered_labels) {
  m_filteredLabels = filtered_labels;
}

QMap<QString, QList<QRectF>>
AnnotationFilter::filter(const QMap<QString, QList<QRectF>> &bboxes) {
  Anntation result;
  if (m_filteredLabels.empty())
    return bboxes;

  const QStringList keys = m_filteredLabels.keys();
  for (const auto &lb : keys) {
    if (bboxes.contains(lb)) {
      for (const auto &box : bboxes[lb]) {
        result[m_filteredLabels[lb]] << box;
      }
    }
  }
  return result;
}
