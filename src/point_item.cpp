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
    : QGraphicsEllipseItem(0, 0, 2 * Helper::kPointRadius,
                           2 * Helper::kPointRadius, parent) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);

  __setLocked(this, !ready);
  if (ready) {
    setSelected(ready);
  }

  __setLabel(this, label);
  setPos(center - QPointF{Helper::kPointRadius, Helper::kPointRadius});
}

void PointItem::setLocked(bool what) { __setLocked(this, what); }

void PointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                      QWidget *widget) {
  QRectF brect = rect();  // boundingRect();
  QPen pp = painter->pen();
  if (!m_moveEnable) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(pen().brush());
    painter->drawEllipse(brect);
  } else {
    painter->setPen(pen());
    float pw = pen().widthF();
    brect.adjust(pw, pw, -pw, -pw);
    painter->drawLine(brect.topLeft(), brect.bottomRight());
    painter->drawLine(brect.bottomLeft(), brect.topRight());
  }
  if (m_showLabel) {
    painter->setFont(Helper::fontLabel());
    painter->setPen(Qt::black);
    brect = boundingRect();
    QRectF lb_rect(brect.x(), brect.y(), m_labelLen, Helper::kLabelRectH);
    painter->fillRect(lb_rect, Helper::kLabelColor);
    painter->drawText(lb_rect, Qt::AlignVCenter, " " + m_label);
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
    EditDialog dlg;
    dlg.setGeometry(QRect{event->screenPos(), dlg.size()});
    dlg.setLabel(m_label);
    if (dlg.exec() == QDialog::Accepted) {
      if (dlg.label() != m_label) {
        setLabel(dlg.label());
        emit dynamic_cast<ImageCanvas *>(scene())->needSaveChanges();
      }
    }
  } else {
    QGraphicsEllipseItem::mousePressEvent(event);
  }
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

QRectF PointItem::boundingRect() const {
  QRectF br = QGraphicsEllipseItem::boundingRect();
  double dw = 0;
  if (rect().width() < m_labelLen) {
    dw = m_labelLen - rect().width();
  }
  return rect().adjusted(-0.5, -Helper::kLabelRectH, dw + 0.5, 0.5);
}

QPointF PointItem::center() const {
  const QSizeF sf = rect().size();
  return pos() + QPointF{0.5 * sf.width(), 0.5 * sf.height()};
}
