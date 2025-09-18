#ifndef UTILS_H
#define UTILS_H
#include <QAction>
#include <QColor>
#include <QDir>
#include <QFont>
#include <QGraphicsItem>
#include <QStringList>

class ImageCanvas;
class LabelTreeModel;

class ProgressValue {
  int m_nSteps{100};
  int m_lastValue{0};
  int m_lastStep{0};

 public:
  explicit ProgressValue(int nsteps) : m_nSteps{nsteps} {}
  int value() {
    // qDebug() << m_nSteps << m_lastValue << m_lastStep;
    m_lastStep += 1;
    const int v = static_cast<int>((100.0 * m_lastStep) / m_nSteps);
    if (v > m_lastValue) {
      m_lastValue = v;
      return std::min(m_lastValue, 100);
    } else {
      return -1;
    }
  }
};

class Helper {
  static QFont m_fontLabel;
  static QMap<QString, QColor> m_labelToColor;
  static ImageCanvas *m_scene;
  static QColor circleColor;
  static QColor circleColorSelected;

 public:
  static bool m_labelsUpdated;
  static double kPointRadius;
  static int kFontPixelSize;
  static double kInvScaleFactor;
  static double kLineWidth;
  static QStringList kImgExts;
  static LabelTreeModel *labelTreeModel;

  static const QString organizationName;
  static const QString organizationDomain;
  static const QString appName;

  enum CustomItemType {
    kBBox = QGraphicsItem::UserType + 1,
    kCircle,
    kLine,
    kPoint,
    kPolygon,
    kLineStrip,
    kNoItem,
  };

  Helper() = default;

  static int64_t seconsToYear5000(
      const std::optional<QDateTime> &dt = std::nullopt);
  static void InitFonts(const QFont &baseFont);
  static void InitSupportedImageFormats();
  static void setScale(const double &scale);
  static void setImageCanvas(ImageCanvas *scene);
  static ImageCanvas *imageCanvas() { return m_scene; }
  static const QFont &fontLabel();
  static double penWidth();
  static QColor getCircleColor(bool selected);
  static QColor getUnlockedColor(bool selected);

  static void registerNewLabels(const QStringList &labels);
  static void clearLabels();
  static QStringList currentLabels() { return Helper::m_labelToColor.keys(); }

  static QRectF buildRectFromTwoPoints(const QPointF &p1, const QPointF &p2);
  static QColor colorFromLabel(const QString &label);
  // static QColor colorFromText(const QString &text);

  static double pointLen(const QPointF &p);
  static double distanceToLine(const QVector2D &lp1, const QVector2D &lp2,
                               const QPointF &p);
  static QPointF pointLineIntersection(const QVector2D &lp1,
                                       const QVector2D &lp2, const QPointF &p);
  static void drawCircleOrSquared(QPainter *painter, const QPointF &ct, qreal w,
                                  bool circle);
  static double polygonArea(const QPolygonF &poly);
  static QPointF intermediatePoint(const QPointF &p1, const QPointF &p2,
                                   double dist);

  static const QColor kUnlockedBBoxColor;
  static const QColor kUnlockedBBoxColorSelected;
  static const QColor kLockedBBoxColor;
  static const QColor kMarginBBoxColor;
  static const QColor kLabelColor;
  static const double kMinPenW;
  static const double kMaxPenW;
  static const int kImageMarging;
  static const double kBorderSize;
};
#endif  // UTILS_H
