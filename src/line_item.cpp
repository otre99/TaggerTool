#include "line_item.h"

#include <QCursor>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <cmath>

#include "imagecanvas.h"
#include "undo_cmds.h"

LineItem::LineItem(ImageCanvas *canvas, const QPointF &p1, const QPointF &p2,
                   const QString &label, const QString &dsc,
                   QGraphicsItem *parent, bool ready)
    : QGraphicsLineItem(p1.x(), p1.y(), p2.x(), p2.y(), parent) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);
  m_canvas = canvas;
  m_description = dsc;
  __setLocked(this, !ready);
  if (ready) {
    setSelected(ready);
  }

  m_label = label;
  setPen(Helper::colorFromLabel(m_label));
  QFontMetrics fm(Helper::fontLabel());
  m_labelLen = fm.horizontalAdvance(m_label);
  m_labelHeight = fm.height();

  auto p = pen();
  p.setWidthF(2 * Helper::penWidth());
  setPen(p);
  // setAcceptHoverEvents(true);
}

void LineItem::helperParametersChanged() {
  prepareGeometryChange();
  __calculateLabelSize(m_label);
  QPen p = pen();
  p.setWidthF(2 * Helper::penWidth());
  setPen(p);
}

void LineItem::setLabel(const QString &lb) {
  m_label = lb;
  auto p = pen();
  p.setColor(Helper::colorFromLabel(lb));
  this->setPen(p);
  QFontMetrics fm(Helper::fontLabel());
  m_labelLen = fm.horizontalAdvance(m_label);
}

// QGraphicsItem
void LineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget) {
  (void)widget;
  QPen p = pen();
  p.setWidthF(p.widthF() / 2.0);
  painter->setPen(p);
  if (!m_editEnable) {
    QPen pp = p;
    pp.setWidth(Helper::kLineWidth);
    pp.setCosmetic(true);
    painter->setPen(pp);
    painter->drawLine(line());
    painter->setPen(p);
  } else {
    painter->save();
    auto pp = p;
    pp.setWidthF(Helper::kLineWidth);
    pp.setCosmetic(true);
    pp.setColor(Qt::black);
    painter->setPen(pp);
    painter->drawLine(line());
    painter->restore();

    painter->setPen(Qt::NoPen);
    painter->setBrush(Helper::getCircleColor(false));

    const QPair<CORNER, QPointF> pts[] = {
        {kP1, line().p1()},
        {kP2, line().p2()},
    };

    for (auto &&[corner, pt] : pts) {
      if (m_currentCorner == corner) {
        painter->save();
        painter->setBrush(Helper::getCircleColor(true));
        Helper::drawCircleOrSquared(painter, pt, p.widthF(),
                                    m_currentCorner != corner);
        painter->restore();
      } else {
        Helper::drawCircleOrSquared(painter, pt, p.widthF(),
                                    m_currentCorner != corner);
      }
    }
  }

  if (m_canvas->showLabels()) {
    painter->setFont(Helper::fontLabel());
    painter->setPen(Qt::black);
    QRectF brect = QGraphicsLineItem::boundingRect();
    QRectF lb_rect(brect.center().x(), brect.center().y() - m_labelHeight,
                   m_labelLen, m_labelHeight);
    painter->fillRect(lb_rect, Helper::kLabelColor);
    painter->drawText(lb_rect, Qt::AlignVCenter, m_label);
  }
}

void LineItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  if (m_currentCorner == kCenter || !m_editEnable)
    QGraphicsItem::mouseMoveEvent(event);
  else {
    QPointF cpos = event->pos();
    if (m_currentCorner == kP1) {
      auto p = line().p2();
      setLine(cpos.x(), cpos.y(), p.x(), p.y());
    } else {
      auto p = line().p1();
      setLine(p.x(), p.y(), cpos.x(), cpos.y());
    }
  }
}

void LineItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  m_currentCorner = positionInside(event->pos());
  // Only the press that starts the gesture may set the baseline, and it must be
  // set before showEditDialog() runs its nested event loop. A second button
  // pressed mid-drag would otherwise rebase it to the already-modified geometry
  // and the change in flight would never reach the undo stack.
  if (!m_gestureActive) {
    m_oldLine = line();
    m_oldPos = pos();
    m_gestureActive = true;
  }
  if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) &&
      event->button() == Qt::LeftButton) {
    __swapStackOrder(this, scene()->items(event->scenePos()));
  } else if (event->modifiers() == Qt::ShiftModifier &&
             event->button() == Qt::LeftButton) {
    setLocked(m_editEnable);
  } else if (event->button() == Qt::RightButton && m_editEnable) {
    showEditDialog(this, event->screenPos());
  } else {
    if (m_currentCorner == kCenter && m_editEnable) {
      setCursor(Qt::DragMoveCursor);
      QGraphicsLineItem::mousePressEvent(event);
    } else {
      setCursor(Qt::ArrowCursor);
      update();
    }
  }
  update();
}

void LineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  setCursor(Qt::ArrowCursor);
  QGraphicsLineItem::mouseReleaseEvent(event);

  // Without an in-flight gesture there is no valid baseline to compare against.
  if (!m_gestureActive) {
    m_currentCorner = kInvalid;
    update();
    return;
  }
  m_gestureActive = false;

  if (m_oldLine != line()) {
    Helper::imageCanvas()->undoStack()->push(
        new ChangeLineSizeCommand(m_oldLine, line(), this, nullptr));
    m_oldLine = line();
  }

  if (m_oldPos != pos()) {
    Helper::imageCanvas()->undoStack()->push(
        new MoveItemCommand(m_oldPos, pos(), this, nullptr));
    m_oldPos = pos();
  }
  m_currentCorner = kInvalid;
  update();
}

QRectF LineItem::boundingRect() const {
  QRectF br = QGraphicsLineItem::boundingRect();
  double dw = 0;
  double dh = 0;
  if (0.5 * br.width() < m_labelLen) {
    dw = m_labelLen - 0.5 * br.width();
  }
  if (0.5 * br.height() < m_labelHeight) {
    dh = m_labelHeight - 0.5 * br.height();
  }
  qreal w = pen().widthF() / 2.0;
  return br.adjusted(-w / 2, -dh - w, dw + w, w);
}

// private
LineItem::CORNER LineItem::positionInside(const QPointF &pos) {
  qreal d1 = Helper::pointLen(line().p1() - pos);
  qreal d2 = Helper::pointLen(line().p2() - pos);
  qreal th = pen().widthF() / 2.0;
  if (d1 < th) return kP1;
  if (d2 < th) return kP2;
  return kCenter;
}
