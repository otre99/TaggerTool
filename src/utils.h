#ifndef UTILS_H
#define UTILS_H
#include <QStringList>
#include <QDir>
#include <QColor>

class Helper {

public:
    static QMap<QString, QList<QRectF>> ReadCSVSingleFile(const QString &path);
    static QMap<QString, QList<QRectF>> ReadXMLingleFile(const QString &path);
    static void SaveCSVSingleFile(const QString &path, const QMap<QString, QList<QRectF>> &bboxes, const QString &img_id, const QSize &img_size);
    static void SaveXMLSingleFile(const QString &path, const QMap<QString, QList<QRectF>> &bboxes, const QString &img_id, const QSize &img_size);
    static QRectF BuildRectFromTwoPoints(const QPointF &p1, const QPointF &p2);
    static QColor ColorFromLabel(const QString &label);

private:
    static QMap<QString, QColor> lb2colors_;
};
#endif // UTILS_H
