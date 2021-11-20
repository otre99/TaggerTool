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

double pointLeng(const QPointF &p) {
  return std::sqrt(p.x() * p.x() + p.y() * p.y());
}

LineItem::LineItem(const QPointF &p1, const QPointF &p2, const QString &label,
                   QGraphicsItem *parent, bool ready)
    : QGraphicsLineItem(p1.x(), p1.y(), p2.x(), p2.y(), parent) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);

  m_moveEnable = ready;
  Helper::setLocked(this, m_moveEnable);
  if (ready) {
    setSelected(ready);
  }

  m_label = label;
  setPen(Helper::colorFromLabel(m_label));
  QFontMetrics fm(Helper::fontLabel());
  m_labelLen = fm.horizontalAdvance(m_label);

  auto p = pen();
  p.setWidthF(2 * Helper::kPointRadius);
  setPen(p);
  setAcceptHoverEvents(true);
}

void LineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget) {
  (void)widget;
  QPen p = pen();
  p.setWidth(Helper::kPointRadius / 2);
  painter->setPen(p);
  painter->drawLine(line());
  if (m_moveEnable) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(pen().color());

    if (m_currentCorner==kP1) {
        painter->save();
        painter->setPen(p);
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(line().p1(), Helper::kPointRadius,
                             Helper::kPointRadius);
        painter->restore();
    } else {
    painter->drawEllipse(line().p1(), Helper::kPointRadius,
                         Helper::kPointRadius);
    }

    if (m_currentCorner==kP2){
        painter->save();
        painter->setPen(p);
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(line().p2(), Helper::kPointRadius,
                             Helper::kPointRadius);
        painter->restore();
    } else {
    painter->drawEllipse(line().p2(), Helper::kPointRadius,
                         Helper::kPointRadius);
    }
  }
  if (m_showLabel) {
    painter->setFont(Helper::fontLabel());
    painter->setPen(Qt::black);
    QRectF brect = QGraphicsLineItem::boundingRect();
    QRectF lb_rect(brect.center().x(), brect.center().y() - Helper::kLabelRectH,
                   m_labelLen, Helper::kLabelRectH);
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
    QPointF pt = mapToScene(event->pos());
    const auto l = scene()->items(pt);
    if (l.count() > 1) {
      for (int i = 1; i < l.count(); ++i) this->stackBefore(l[i]);
      auto *citem = dynamic_cast<CustomItem *>(l[1]);
      if (citem) {
        citem->setLocked(false);
        this->setLocked(true);
      }
    }
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
  m_currentCorner=kInvalid;
  update();
}

LineItem::CORNER LineItem::positionInsideBBox(const QPointF &pos) {
  qreal d1 = pointLeng(line().p1() - pos);
  qreal d2 = pointLeng(line().p2() - pos);

  if (d1 < Helper::kPointRadius) return kP1;

  if (d2 < Helper::kPointRadius) return kP2;
  return kCenter;
}

void LineItem::keyPressEvent(QKeyEvent *event) {
  QGraphicsLineItem::keyPressEvent(event);
}

void LineItem::setLocked(bool what) {
  CustomItem::setLocked(what);
  Helper::setLocked(this, m_moveEnable);
}

QVariant LineItem::itemChange(QGraphicsItem::GraphicsItemChange change,
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

QRectF LineItem::boundingRect() const {
  QRectF br = QGraphicsLineItem::boundingRect();
  double dw = 0;
  double dh = 0;
  if (0.5 * br.width() < m_labelLen) {
    dw = m_labelLen - 0.5 * br.width();
  }
  if (0.5 * br.height() < Helper::kLabelRectH) {
    dh = Helper::kLabelRectH - 0.5 * br.height();
  }
  return br.adjusted(-0.5, -dh - 0.5, dw + 0.5, 0.5);
}

void LineItem::setLabel(const QString &lb) {
  CustomItem::setLabel(lb);
  setPen(Helper::colorFromLabel(lb));
  auto p = pen();
  p.setWidthF(2 * Helper::kPointRadius);
  setPen(p);
  QFontMetrics fm(Helper::fontLabel());
  m_labelLen = fm.horizontalAdvance(m_label);
}
