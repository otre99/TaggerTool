#ifndef UTILS_H
#define UTILS_H
#include <QAction>
#include <QColor>
#include <QDir>
#include <QFont>
#include <QGraphicsItem>
#include <QStringList>

class Helper {
  static QFont m_fontLabel;
  static QMap<QString, QColor> m_labelToColor;

 public:
  enum CustomItemType { kBBox = QGraphicsItem::UserType + 1, kLine, kPoint, kPolygon};

  Helper() = default;
  static void InitFonts(const QFont &baseFont);
  static const QFont &fontLabel();

  static QStringList currentLabels() { return Helper::m_labelToColor.keys(); }

  static QRectF buildRectFromTwoPoints(const QPointF &p1, const QPointF &p2);
  static QColor colorFromLabel(const QString &label);
  static void setLocked(QGraphicsItem *item, bool lk);
  static double pointLen(const QPointF &p);

  static const QColor kUnlockedBBoxColor;
  static const QColor kLockedBBoxColor;
  static const QColor kMarginBBoxColor;
  static const QColor kLabelColor;
  static const int kLabelRectH;
  static const double kPointRadius;
  static const int kFontPixelSize;
  static const double kBorderSize;
};
#endif  // UTILS_H
