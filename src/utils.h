#ifndef UTILS_H
#define UTILS_H
#include <QAction>
#include <QColor>
#include <QDir>
#include <QFont>
#include <QGraphicsItem>
#include <QStringList>

class ImageCanvas;

class Helper {
  static QFont m_fontLabel;
  static QMap<QString, QColor> m_labelToColor;
  static ImageCanvas *m_scene;

 public:
  static bool m_labelsUpdated;
  static double kPointRadius;
  static int kFontPixelSize;
  static double kInvScaleFactor;
  static double kLineWidth;
  static QStringList kImgExts;

  enum CustomItemType {
    kBBox = QGraphicsItem::UserType + 1,
    kLine,
    kPoint,
    kPolygon,
    kLineStrip
  };

  Helper() = default;
  static void InitFonts(const QFont &baseFont);
  static void InitSupportedImageFormats();
  static void setScale(const double &scale);
  static void setImageCanvas(ImageCanvas *scene);
  static ImageCanvas *imageCanvas() { return m_scene; }
  static const QFont &fontLabel();
  static double penWidth();

  static void registerNewLabels(const QStringList &labels);
  static void clearLabels();
  static QStringList currentLabels() { return Helper::m_labelToColor.keys(); }
  static QRectF buildRectFromTwoPoints(const QPointF &p1, const QPointF &p2);
  static QColor colorFromLabel(const QString &label);
  static double pointLen(const QPointF &p);
  static double distanceToLine(const QVector2D &lp1, const QVector2D &lp2,
                               const QPointF &p);
  static QPointF pointLineIntersection(const QVector2D &lp1,
                                       const QVector2D &lp2, const QPointF &p);
  static void drawCircleOrSquared(QPainter *painter, const QPointF &ct, qreal w,
                                  bool circle);
  static const QRgb kLabelColorsArray[80];
  static const QColor kUnlockedBBoxColor;
  static const QColor kLockedBBoxColor;
  static const QColor kMarginBBoxColor;
  static const QColor kLabelColor;
  static const double kMinPenW;
  static const double kMaxPenW;
  static const int kImageMarging;
  static const double kBorderSize;
};
#endif  // UTILS_H
