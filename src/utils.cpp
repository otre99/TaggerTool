#include "utils.h"

#include <QDebug>
#include <QRectF>
#include <QTextStream>
#include <cmath>

const QColor Helper::kUnlockedBBoxColor = {0, 0, 128, 64};
const QColor Helper::kLockedBBoxColor = {128, 128, 128, 64};
const QColor Helper::kMarginBBoxColor = {128, 0, 0, 64};
const QColor Helper::kLabelColor = {200, 200, 200, 64};
const int Helper::kLabelRectH = 11;
const int Helper::kFontPixelSize = 9;
const double Helper::kBorderSize = 32;
const double Helper::kPointRadius = 4;

QMap<QString, QColor> Helper::m_labelToColor{};
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
  if (Helper::m_labelToColor.contains(label)) {
    return Helper::m_labelToColor[label];
  }
  const QStringList lst = QColor::colorNames();
  int index = std::hash<std::string>()(label.toStdString()) % lst.count();
  Helper::m_labelToColor[label] = lst[index];
  return Helper::m_labelToColor[label];
}

void Helper::setLocked(QGraphicsItem *item, bool lk) {
  item->setFlag(QGraphicsItem::ItemIsMovable, lk);
  item->setFlag(QGraphicsItem::ItemIsSelectable, lk);
  item->setFlag(QGraphicsItem::ItemIsFocusable, lk);
}

double Helper::pointLen(const QPointF &p) {
  return std::sqrt(p.x() * p.x() + p.y() * p.y());
}
