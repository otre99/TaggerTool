#include "point_item.h"

#include <QAction>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QMenu>
#include <QPainter>
#include <QPen>

#include "editdialog.h"
#include "imagecanvas.h"
#include "utils.h"

PointItem::PointItem(const QPointF &center, const QString &label,
                     QGraphicsItem *parent, bool ready)
    : QGraphicsEllipseItem(0, 0, 2 * Helper::penWidth(), 2 * Helper::penWidth(),
                           parent) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);

  __setLocked(this, !ready);
  if (ready) {
    setSelected(ready);
  }

  __setLabel(this, label);
  setPos(center - QPointF{Helper::penWidth(), Helper::penWidth()});
  QPen p = pen();
  p.setWidthF(Helper::penWidth() / 2.0);
  setPen(p);
}

// CustomItem
void PointItem::helperParametersChanged() {
  prepareGeometryChange();
  __calculateLabelSize(m_label);
  auto pt = rect().center() - QPointF{Helper::penWidth(), Helper::penWidth()};
  setRect(pt.x(), pt.y(), 2 * Helper::penWidth(), 2 * Helper::penWidth());
  QPen p = pen();
  p.setWidthF(Helper::penWidth() / 2.0);
  setPen(p);
}

// QGraphicsItem
void PointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                      QWidget *widget) {
  QRectF brect = rect();  // boundingRect();
  if (!m_moveEnable) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(pen().brush());
    painter->drawEllipse(brect);
  } else {
    painter->setPen(pen());
    float pw = pen().widthF();
    brect.adjust(pw / 2.0, pw / 2.0, -pw / 2, -pw / 2);
    painter->drawLine(brect.topLeft(), brect.bottomRight());
    painter->drawLine(brect.bottomLeft(), brect.topRight());
  }
  if (m_showLabel) {
    painter->setFont(Helper::fontLabel());
    painter->setPen(Qt::black);
    brect = boundingRect();
    QRectF lb_rect(brect.x(), brect.y(), m_labelLen, m_labelHeight);
    painter->fillRect(lb_rect, Helper::kLabelColor);
    painter->drawText(lb_rect, Qt::AlignVCenter, m_label);
  }
}

void PointItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) &&
      event->button() == Qt::LeftButton) {
    __swapStackOrder(this, scene()->items(event->scenePos()));
  } else if (event->modifiers() == Qt::ShiftModifier &&
             event->button() == Qt::LeftButton) {
    setLocked(m_moveEnable);
  } else if (event->button() == Qt::RightButton && m_moveEnable) {
    __showEditDialog(this, event->screenPos());
  } else {
    QGraphicsEllipseItem::mousePressEvent(event);
  }
}

QRectF PointItem::boundingRect() const {
  QRectF br = QGraphicsEllipseItem::boundingRect();
  double dw = 0;
  if (br.width() < m_labelLen) {
    dw = m_labelLen - br.width();
  }
  qreal o = pen().widthF() / 2.0;
  return rect().adjusted(-o, -o - m_labelHeight, dw + o, o);
}

QVariant PointItem::itemChange(QGraphicsItem::GraphicsItemChange change,
                               const QVariant &value) {
  if (scene() == nullptr) return value;
  switch (change) {
    case QGraphicsItem::ItemPositionChange:
      emit dynamic_cast<ImageCanvas *>(scene())->needSaveChanges();
      break;
    default:
      break;
  }
  return value;
}

// get/set
QPointF PointItem::center() const { return mapToScene(rect().center()); }
