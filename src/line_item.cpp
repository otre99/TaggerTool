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

#include "editdialog.h"
#include "imagecanvas.h"

LineItem::LineItem(const QPointF &p1, const QPointF &p2, const QString &label,
                   QGraphicsItem *parent, bool ready)
    : QGraphicsLineItem(p1.x(), p1.y(), p2.x(), p2.y(), parent) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);

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
  p.setWidthF(2 * Helper::kPointRadius * Helper::kInvScaleFactor);
  setPen(p);
  setAcceptHoverEvents(true);
}

void LineItem::helperParametersChanged() {
  prepareGeometryChange();
  __calculateLabelSize(m_label);
  QPen p = pen();
  p.setWidthF(2 * Helper::kPointRadius * Helper::kInvScaleFactor);
  setPen(p);
}

void LineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget) {
  (void)widget;
  QPen p = pen();
  p.setWidthF(p.widthF() / 2.0);
  painter->setPen(p);
  if (!m_moveEnable)
    painter->drawLine(line());

  if (m_moveEnable) {
    painter->save();
    auto pp = p;
    pp.setWidth(1);
    pp.setStyle(Qt::DotLine);
    pp.setColor(Qt::black);
    painter->setPen(pp);
    painter->drawLine(line());
    painter->restore();

    painter->setPen(Qt::NoPen);
    QColor color = pen().color();
    color.setAlpha(150);
    painter->setBrush(color);

    Helper::drawCircleOrSquared(painter, line().p1(), p.widthF(),
                                m_currentCorner != kP1);
    Helper::drawCircleOrSquared(painter, line().p2(), p.widthF(),
                                m_currentCorner != kP2);
  }

  if (m_showLabel) {
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
  if (m_currentCorner == kCenter || !m_moveEnable)
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
    emit dynamic_cast<ImageCanvas *>(scene())->needSaveChanges();
  }
  if (isSelected())
    emit dynamic_cast<ImageCanvas *>(scene())->bboxItemToEditor(this, 0);
}

void LineItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
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
    m_currentCorner = positionInsideBBox(event->pos());
    if (m_currentCorner == kCenter || !m_moveEnable) {
      QGraphicsLineItem::mousePressEvent(event);
    } else {
      update();
    }
  }
}

void LineItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
  QGraphicsLineItem::mouseDoubleClickEvent(event);
}

void LineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  setCursor(Qt::ArrowCursor);
  QGraphicsLineItem::mouseReleaseEvent(event);
  if (isSelected())
    emit dynamic_cast<ImageCanvas *>(scene())->bboxItemToEditor(this, 0);
  m_currentCorner = kInvalid;
  update();
}

LineItem::CORNER LineItem::positionInsideBBox(const QPointF &pos) {
  qreal d1 = Helper::pointLen(line().p1() - pos);
  qreal d2 = Helper::pointLen(line().p2() - pos);
  qreal th = pen().widthF() / 2.0;
  if (d1 < th)
    return kP1;
  if (d2 < th)
    return kP2;
  return kCenter;
}

void LineItem::keyPressEvent(QKeyEvent *event) {
  QGraphicsLineItem::keyPressEvent(event);
}

void LineItem::setLocked(bool lk) { __setLocked(this, lk); }

QVariant LineItem::itemChange(QGraphicsItem::GraphicsItemChange change,
                              const QVariant &value) {
  if (scene() == nullptr)
    return value;
  switch (change) {
  case QGraphicsItem::ItemPositionChange:
    emit dynamic_cast<ImageCanvas *>(scene())->needSaveChanges();
    break;
  default:
    break;
  }
  return value;
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

void LineItem::setLabel(const QString &lb) {
  m_label = lb;
  auto p = pen();
  p.setColor(Helper::colorFromLabel(lb));
  this->setPen(p);
  QFontMetrics fm(Helper::fontLabel());
  m_labelLen = fm.horizontalAdvance(m_label);
}
