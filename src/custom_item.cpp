#include "custom_item.h"
#include <QFontMetrics>
#include <QAbstractGraphicsShapeItem>
#include <QDebug>
#include <QPen>

#include "utils.h"

void CustomItem::__setLabel(QAbstractGraphicsShapeItem *item, QString label) {
  m_label = label;
  auto p = item->pen();
  p.setColor(Helper::colorFromLabel(label));
  item->setPen(p);
  __calculateLabelSize(label);
}

void CustomItem::__calculateLabelSize(const QString &label) {
  QFontMetrics fm(Helper::fontLabel());
  m_labelLen = fm.horizontalAdvance(label);
  m_labelHeight = fm.height();
}

void CustomItem::__setLocked(QGraphicsItem *item, bool lk) {
  m_moveEnable = !lk;
  item->setFlag(QGraphicsItem::ItemIsMovable, m_moveEnable);
  item->setFlag(QGraphicsItem::ItemIsSelectable, m_moveEnable);
  item->setFlag(QGraphicsItem::ItemIsFocusable, m_moveEnable);
}

void CustomItem::__swapStackOrder(QGraphicsItem *item,
                                  const QList<QGraphicsItem *> &l) {
  if (l.count() > 1) {
    for (int i = 1; i < l.count(); ++i) item->stackBefore(l[i]);
    auto *citem = dynamic_cast<CustomItem *>(l[1]);
    if (citem) {
      citem->setLocked(false);
      dynamic_cast<CustomItem *>(item)->setLocked(true);
    }
  }
}
