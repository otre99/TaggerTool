#include "utils.h"

#include <QDebug>
#include <QImageReader>
#include <QPainter>
#include <QRectF>
#include <QTextStream>
#include <QVector2D>
#include <QVector3D>
#include <cmath>

const QRgb Helper::kLabelColorsArray[] = {
    qRgb(0, 113, 188),   qRgb(216, 82, 24),   qRgb(236, 176, 31),
    qRgb(125, 46, 141),  qRgb(118, 171, 47),  qRgb(76, 189, 237),
    qRgb(161, 19, 46),   qRgb(76, 76, 76),    qRgb(153, 153, 153),
    qRgb(255, 0, 0),     qRgb(255, 127, 0),   qRgb(190, 190, 0),
    qRgb(0, 255, 0),     qRgb(0, 0, 255),     qRgb(170, 0, 255),
    qRgb(84, 84, 0),     qRgb(84, 170, 0),    qRgb(84, 255, 0),
    qRgb(170, 84, 0),    qRgb(170, 170, 0),   qRgb(170, 255, 0),
    qRgb(255, 84, 0),    qRgb(255, 170, 0),   qRgb(255, 255, 0),
    qRgb(0, 84, 127),    qRgb(0, 170, 127),   qRgb(0, 255, 127),
    qRgb(84, 0, 127),    qRgb(84, 84, 127),   qRgb(84, 170, 127),
    qRgb(84, 255, 127),  qRgb(170, 0, 127),   qRgb(170, 84, 127),
    qRgb(170, 170, 127), qRgb(170, 255, 127), qRgb(255, 0, 127),
    qRgb(255, 84, 127),  qRgb(255, 170, 127), qRgb(255, 255, 127),
    qRgb(0, 84, 255),    qRgb(0, 170, 255),   qRgb(0, 255, 255),
    qRgb(84, 0, 255),    qRgb(84, 84, 255),   qRgb(84, 170, 255),
    qRgb(84, 255, 255),  qRgb(170, 0, 255),   qRgb(170, 84, 255),
    qRgb(170, 170, 255), qRgb(170, 255, 255), qRgb(255, 0, 255),
    qRgb(255, 84, 255),  qRgb(255, 170, 255), qRgb(84, 0, 0),
    qRgb(127, 0, 0),     qRgb(170, 0, 0),     qRgb(212, 0, 0),
    qRgb(255, 0, 0),     qRgb(0, 42, 0),      qRgb(0, 84, 0),
    qRgb(0, 127, 0),     qRgb(0, 170, 0),     qRgb(0, 212, 0),
    qRgb(0, 255, 0),     qRgb(0, 0, 42),      qRgb(0, 0, 84),
    qRgb(0, 0, 127),     qRgb(0, 0, 170),     qRgb(0, 0, 212),
    qRgb(0, 0, 255),     qRgb(0, 0, 0),       qRgb(36, 36, 36),
    qRgb(72, 72, 72),    qRgb(109, 109, 109), qRgb(145, 145, 145),
    qRgb(182, 182, 182), qRgb(218, 218, 218), qRgb(0, 113, 188),
    qRgb(80, 182, 188),  qRgb(127, 127, 0)};

const QColor Helper::kUnlockedBBoxColor = {0, 0, 128, 64};
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

QMap<QString, QColor> Helper::m_labelToColor{};
ImageCanvas *Helper::m_scene = nullptr;
bool Helper::m_labelsUpdated = false;
QFont Helper::m_fontLabel;

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

QColor Helper::colorFromLabel(const QString &label) {
  if (label.isEmpty())
    return Qt::black;

  if (Helper::m_labelToColor.contains(label)) {
    return Helper::m_labelToColor[label];
  }
  m_labelsUpdated = true;
  const int index = std::hash<std::string>()(label.toStdString()) % 80;
  Helper::m_labelToColor[label] = kLabelColorsArray[index];
  return Helper::m_labelToColor[label];
}

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
