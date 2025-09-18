#include "utils.h"

#include <QDebug>
#include <QImageReader>
#include <QPainter>
#include <QRectF>
#include <QTextStream>
#include <QVector2D>
#include <QVector3D>
#include <cmath>

const QString Helper::organizationName = "RCCR";
const QString Helper::appName = "CVTaggerTool";
const QString Helper::organizationDomain = "rccr1987.com";

#include <QCryptographicHash>

const QColor Helper::kUnlockedBBoxColor = {0, 0, 128, 64};
const QColor Helper::kUnlockedBBoxColorSelected = {0, 0, 128, 128};
const QColor Helper::kLockedBBoxColor = {128, 128, 128, 64};
const QColor Helper::kMarginBBoxColor = {128, 0, 0, 64};
const QColor Helper::kLabelColor = {200, 200, 200, 64};
const double Helper::kMinPenW = 0.250;
const double Helper::kMaxPenW = 100.0;

int Helper::kFontPixelSize = 24;
const int Helper::kImageMarging = 32;
const double Helper::kBorderSize = 32;
double Helper::kPointRadius = 8;
double Helper::kInvScaleFactor = 1.0;
double Helper::kLineWidth = 2.0;
QStringList Helper::kImgExts = {};
LabelTreeModel *Helper::labelTreeModel = nullptr;

QMap<QString, QColor> Helper::m_labelToColor{};
ImageCanvas *Helper::m_scene = nullptr;
bool Helper::m_labelsUpdated = false;
QFont Helper::m_fontLabel;

QColor Helper::circleColor{Qt::red};
QColor Helper::circleColorSelected{Qt::green};

int64_t Helper::seconsToYear5000(const std::optional<QDateTime> &dt) {
  const QDateTime year5000 = dt.has_value()
                                 ? dt.value()
                                 : QDateTime(QDate(5000, 1, 1), QTime(0, 0, 0));
  return QDateTime::currentDateTime().secsTo(year5000);
}

void Helper::InitFonts(const QFont &baseFont) {
  Helper::m_fontLabel = baseFont;
  Helper::m_fontLabel.setPixelSize(
      qMax(2.0, Helper::kFontPixelSize * kInvScaleFactor));
  Helper::m_fontLabel.setBold(true);
}

void Helper::InitSupportedImageFormats() {
  kImgExts.clear();
  const auto imgFmt = QImageReader::supportedImageFormats();
  for (const QByteArray &fmt : imgFmt) {
    QString wc(fmt);
    kImgExts.append("*." + wc);
    kImgExts.append("*." + wc.toUpper());
  }
}

void Helper::setScale(const double &scale) {
  kInvScaleFactor = scale;
  Helper::m_fontLabel.setPixelSize(
      qMax(2.0, Helper::kFontPixelSize * kInvScaleFactor));
}
void Helper::setImageCanvas(ImageCanvas *scene) { m_scene = scene; }

const QFont &Helper::fontLabel() { return Helper::m_fontLabel; }

double Helper::penWidth() {
  return qMin(kMaxPenW, qMax(kMinPenW, kPointRadius * kInvScaleFactor));
}

QColor Helper::getCircleColor(bool selected) {
  return selected ? Helper::circleColorSelected : Helper::circleColor;
}

QColor Helper::getUnlockedColor(bool selected) {
  return selected ? Helper::kUnlockedBBoxColorSelected
                  : Helper::kUnlockedBBoxColor;
}

void Helper::registerNewLabels(const QStringList &labels) {
  for (const auto &lb : labels) {
    (void)colorFromLabel(lb);
  }
}
void Helper::clearLabels() { m_labelToColor.clear(); }

QRectF Helper::buildRectFromTwoPoints(const QPointF &p1, const QPointF &p2) {
  double rw = qAbs(p2.x() - p1.x());
  double rh = qAbs(p2.y() - p1.y());
  QPointF topleft(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y()));
  return {topleft, QSizeF(rw, rh)};
}

QColor Helper::colorFromLabel(const QString &text) {
  auto &&iter = Helper::m_labelToColor.find(text);
  if (iter != Helper::m_labelToColor.end()) {
    return *iter;
  }

  QByteArray key = text.toUtf8();

  // Use a stable hash (MD5 gives 128 bits, stable across runs)
  QByteArray md5 = QCryptographicHash::hash(key, QCryptographicHash::Md5);

  // Map first byte to hue (0..359)
  int hue = static_cast<unsigned char>(md5[0]) % 360;

  // Map second byte to saturation (60..90 %)
  int sat = 150 + (static_cast<unsigned char>(md5[1]) % 80);

  // Map third byte to lightness (40..70 %)
  int light = 120 + (static_cast<unsigned char>(md5[2]) % 80);

  QColor c;
  c.setHsl(hue, sat, light);
  Helper::m_labelToColor[text] = c;
  return c;
}

// QColor Helper::colorFromLabel(const QString &label) {
//   if (label.isEmpty()) return Qt::black;

//   if (Helper::m_labelToColor.contains(label)) {
//     return Helper::m_labelToColor[label];
//   }
//   m_labelsUpdated = true;
//   const int index = std::hash<std::string>()(label.toStdString()) % 80;
//   Helper::m_labelToColor[label] = kLabelColorsArray[index];
//   return Helper::m_labelToColor[label];
// }

double Helper::pointLen(const QPointF &p) {
  return std::sqrt(p.x() * p.x() + p.y() * p.y());
}
double Helper::distanceToLine(const QVector2D &lp1, const QVector2D &lp2,
                              const QPointF &p) {
  QVector3D l = QVector3D::crossProduct({lp1, 1}, {lp2, 1});
  return qAbs(l.x() * p.x() + l.y() * p.y() + l.z()) /
         Helper::pointLen({l.x(), l.y()});
}

QPointF Helper::pointLineIntersection(const QVector2D &lp1,
                                      const QVector2D &lp2, const QPointF &p) {
  const QVector3D l = QVector3D::crossProduct({lp1, 1}, {lp2, 1});
  const QVector3D lp(l.y(), -l.x(), -(l.y() * p.x() - l.x() * p.y()));
  QVector3D xp = QVector3D::crossProduct(l, lp);
  return {xp.x() / xp.z(), xp.y() / xp.z()};
}

void Helper::drawCircleOrSquared(QPainter *painter, const QPointF &ct,
                                 qreal radius, bool circle) {
  if (circle) {
    painter->drawEllipse(ct, radius, radius);
  } else {
    painter->drawRect(
        {ct - QPointF(radius, radius), QSizeF{2 * radius, 2 * radius}});
  }
}

double Helper::polygonArea(const QPolygonF &poly) {
  const int n = poly.size();
  if (n < 3) return 0.0;
  long double s = 0;
  for (int i = 0; i < n; ++i) {
    const int j = (i + 1) % n;

    const long double xi = poly[i].x();
    const long double xj = poly[j].x();

    const long double yi = poly[i].y();
    const long double yj = poly[j].y();

    s += xi * yj - xj * yi;
  }
  return std::abs(static_cast<double>(s * 0.5L));
}

QPointF Helper::intermediatePoint(const QPointF &p1, const QPointF &p2,
                                  double dist) {
  auto l = p2 - p1;
  return p1 + dist * l / std::sqrt(l.x() * l.x() + l.y() * l.y());
}
