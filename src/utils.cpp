#include "utils.h"
#include <QDebug>
#include <QRectF>
#include <QTextStream>
#include <cmath>
#include <QVector3D>
#include <QVector2D>
#include <QPainter>


const QColor Helper::kUnlockedBBoxColor = {0, 0, 128, 64};
const QColor Helper::kLockedBBoxColor = {128, 128, 128, 64};
const QColor Helper::kMarginBBoxColor = {128, 0, 0, 64};
const QColor Helper::kLabelColor = {200, 200, 200, 64};
//const int Helper::kLabelRectH = 26;
int Helper::kFontPixelSize = 24;
const int Helper::kImageMarging= 32;
const double Helper::kBorderSize = 32;
double Helper::kPointRadius = 2;

QMap<QString, QColor> Helper::m_labelToColor{};
bool Helper::m_labelsUpdated=false;
QFont Helper::m_fontLabel;

void Helper::InitFonts(const QFont &baseFont) {
  Helper::m_fontLabel = baseFont;
  Helper::m_fontLabel.setPixelSize(Helper::kFontPixelSize);
  Helper::m_fontLabel.setBold(true);
}

const QFont &Helper::fontLabel() { return Helper::m_fontLabel; }

QRectF Helper::buildRectFromTwoPoints(const QPointF &p1, const QPointF &p2) {
  double rw = qAbs(p2.x() - p1.x());
  double rh = qAbs(p2.y() - p1.y());
  QPointF topleft(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y()));
  return {topleft, QSizeF(rw, rh)};
}

QColor Helper::colorFromLabel(const QString &label) {

  if (label.isEmpty()) return Qt::black;

  if (Helper::m_labelToColor.contains(label)) {
    return Helper::m_labelToColor[label];
  }
  m_labelsUpdated=true;
  const QStringList lst = QColor::colorNames();
  int index = std::hash<std::string>()(label.toStdString()) % lst.count();
  Helper::m_labelToColor[label] = lst[index];
  return Helper::m_labelToColor[label];
}

double Helper::pointLen(const QPointF &p) {
  return std::sqrt(p.x() * p.x() + p.y() * p.y());
}
double Helper::distanceToLine(const QVector2D &lp1, const QVector2D &lp2, const QPointF &p)
{
    QVector3D l = QVector3D::crossProduct({lp1,1}, {lp2,1});
    return qAbs(l.x()*p.x()+l.y()*p.y()+l.z())/Helper::pointLen({l.x(), l.y()});
}

QPointF  Helper::pointLineIntersection(const QVector2D &lp1, const QVector2D &lp2, const QPointF &p)
{
    const QVector3D l = QVector3D::crossProduct({lp1,1}, {lp2,1});
    const QVector3D lp(l.y(), -l.x(),-(l.y()*p.x()-l.x()*p.y()));
    QVector3D xp = QVector3D::crossProduct(l,lp);
    return {xp.x()/xp.z(), xp.y()/xp.z()};
}

void   Helper::drawCircleOrSquared(QPainter *painter, const QPointF &ct, qreal w, bool circle){
    if (circle){
        painter->drawEllipse(ct, w,w);
    } else {
        painter->drawRect({ct-QPointF(w, w), QSizeF{2*w, 2*w}});
    }
}
