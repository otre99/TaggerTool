#include "circle_item.h"

#include <QCursor>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>

#include "imagecanvas.h"
#include "undo_cmds.h"

extern Helper globalHelper;

CircleItem::CircleItem(ImageCanvas *canvas, const QPointF &center,
                       double radius, const QString &label, const QString &dsc,
                       QGraphicsItem *parent, bool ready)
    : QGraphicsEllipseItem(0, 0, 2 * radius, 2 * radius, parent) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);

  m_canvas = canvas;
  m_description = dsc;
  setPos(center - QPointF{radius, radius});

  __setLocked(this, !ready);
  if (ready) {
    setSelected(ready);
  }

  __setLabel(this, label);
  auto p = pen();
  p.setWidthF(Helper::penWidth());
  setPen(p);
  //  setAcceptHoverEvents(true);
}

void CircleItem::helperParametersChanged() {
  prepareGeometryChange();
  __calculateLabelSize(m_label);
  QPen p = pen();
  p.setWidthF(Helper::penWidth());
  setPen(p);
}

void CircleItem::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) {
  (void)widget;
  QPen p = pen();
  painter->setPen(p);

  QRectF brect = rect();  // boundingRect();
  if (m_editEnable) {
    painter->setBrush(
        QBrush(Helper::getUnlockedColor(m_currentCorner == kCenter)));
  } else {
    painter->setBrush(QBrush(Helper::kLockedBBoxColor));
  }
  if (!m_editEnable) {
    QPen pp = p;
    pp.setWidthF(Helper::kLineWidth);
    pp.setCosmetic(true);
    painter->setPen(pp);
    painter->drawEllipse(brect);
    painter->setPen(p);
  }
  if (m_editEnable) {
    auto pp = p;
    pp.setCosmetic(true);
    pp.setWidthF(Helper::kLineWidth);
    //        pp.setWidthF(qMin(1.0, p.widthF()));
    pp.setColor(Qt::black);
    painter->setPen(pp);
    painter->drawEllipse(brect);

    const QPointF c = rect().center();
    qreal r = radius();
    painter->drawLine(QPointF{c.x() - r, c.y()}, QPointF{c.x() + r, c.y()});
    painter->drawLine(QPointF{c.x(), c.y() - r}, QPointF{c.x(), c.y() + r});

    if (m_currentCorner == kBorder) {
      painter->setBrush(Helper::getCircleColor(true));

      painter->setPen(pp);
      painter->drawLine(c, m_lastPt);
      painter->setPen(Qt::NoPen);
      qreal radius = p.widthF();
      Helper::drawCircleOrSquared(
          painter,
          Helper::intermediatePoint(c, m_lastPt, this->radius() - radius),
          radius, true);
    }
  }
  if (m_canvas->showLabels()) {
    painter->setFont(globalHelper.fontLabel());
    p.setColor(Qt::black);
    painter->setPen(p);

    brect = boundingRect();
    qreal dx = (brect.width() - m_labelLen) / 2;
    QRectF lb_rect(brect.x() + dx, brect.y(), m_labelLen, m_labelHeight);
    painter->fillRect(lb_rect, Helper::kLabelColor);
    painter->drawText(lb_rect, Qt::AlignVCenter | Qt::AlignHCenter, m_label);
  }
}

void CircleItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  if (m_currentCorner == kCenter || !m_editEnable)
    QGraphicsEllipseItem::mouseMoveEvent(event);
  else {
    const QPointF pos = event->pos();
    const QPointF cpt = rect().center();
    const qreal new_radius = Helper::pointLen(pos - cpt);

    setRect({cpt.x() - new_radius, cpt.y() - new_radius, 2 * new_radius,
             2 * new_radius});
    m_lastPt = pos;
  }
}

void CircleItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  // Capture the old-state baseline for every press, before any branching: the
  // release handler compares against it unconditionally, so a branch that skips
  // the capture (edit dialog, lock toggle) would leave it holding a default
  // constructed QRectF/QPointF and push bogus undo commands on release.
  // Only the press that starts the gesture may set it, so that a second button
  // pressed mid-drag cannot rebase it and swallow the change in flight.
  if (!m_gestureActive) {
    m_oldPos = pos();
    m_oldRect = rect();
    m_gestureActive = true;
  }
  m_currentCorner = kInvalid;
  if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) &&
      event->button() == Qt::LeftButton) {
    __swapStackOrder(this, scene()->items(event->scenePos()));
  } else if (event->modifiers() == Qt::ShiftModifier &&
             event->button() == Qt::LeftButton) {
    setLocked(m_editEnable);
  } else if (event->button() == Qt::RightButton && m_editEnable) {
    showEditDialog(this, event->screenPos());
  } else {
    m_currentCorner = positionInside(event->pos());
    if (m_currentCorner == kCenter && m_editEnable) {
      setCursor(Qt::DragMoveCursor);
      QGraphicsEllipseItem::mousePressEvent(event);
    } else {
      setCursor(Qt::ArrowCursor);
      m_lastPt = event->pos();
      update();
    }
  }
}

void CircleItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
  QGraphicsEllipseItem::mouseDoubleClickEvent(event);
}

void CircleItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  setCursor(Qt::ArrowCursor);
  QGraphicsEllipseItem::mouseReleaseEvent(event);

  // Without an in-flight gesture there is no valid baseline to compare against.
  if (!m_gestureActive) {
    m_currentCorner = kInvalid;
    update();
    return;
  }
  m_gestureActive = false;

  const bool sizeChanged = m_oldRect != rect();
  const bool moved = m_oldPos != pos();
  m_currentCorner = kInvalid;
  if (sizeChanged || moved) {
    auto canvas = dynamic_cast<ImageCanvas *>(this->scene());
    if (sizeChanged) {
      canvas->undoStack()->push(
          new RadiusChangeCircleCommand(m_oldRect, rect(), this));
      m_oldRect = rect();
    }
    if (moved) {
      canvas->undoStack()->push(new MoveItemCommand(m_oldPos, pos(), this));
      m_oldPos = pos();
    }
  }
  update();
}

// void CircleItem::keyPressEvent(QKeyEvent *event) {
//    if (event->key() == Qt::Key_Return) {
//         this->setLocked(true);
//    } else {
//         QGraphicsItem::keyPressEvent(event);
//    }
// }

QRectF CircleItem::boundingRect() const {
  QRectF br = QGraphicsEllipseItem::boundingRect();
  double dw = 0;
  if (br.width() < m_labelLen) {
    dw = m_labelLen - br.width();
  }
  qreal o = pen().widthF() / 2.0;
  return rect().adjusted(-o - dw / 2, -o - m_labelHeight, dw / 2 + o, o);
}

// private
CircleItem::CORNER CircleItem::positionInside(const QPointF &pos) {
  const qreal th = pen().widthF();
  const qreal r = radius();
  const qreal dist = Helper::pointLen(pos - rect().center());
  qreal delta = r - dist;

  if (dist < r - th) return kCenter;

  if (qAbs(delta) < th) return kBorder;

  return kInvalid;
}

QPointF CircleItem::center() const { return mapToScene(rect().center()); }

qreal CircleItem::radius() const { return rect().width() / 2; }
