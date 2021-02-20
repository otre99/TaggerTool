#ifndef UTILS_H
#define UTILS_H
#include <QColor>
#include <QDir>
#include <QStringList>

class Helper {

public:
  static QMap<QString, QList<QRectF>> readCSVSingleFile(const QString &path);
  static QMap<QString, QList<QRectF>> readXMLingleFile(const QString &path);
  static void saveCSVSingleFile(const QString &path,
                                const QMap<QString, QList<QRectF>> &bboxes,
                                const QString &img_id, const QSize &img_size);
  static void saveXMLSingleFile(const QString &path,
                                const QMap<QString, QList<QRectF>> &bboxes,
                                const QString &img_id, const QSize &img_size);
  static QRectF buildRectFromTwoPoints(const QPointF &p1, const QPointF &p2);
  static QColor colorFromLabel(const QString &label);
};
#endif // UTILS_H
