#include "polygon_item.h"

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


PolygonItem::PolygonItem(const QPolygonF &poly, const QString &label, QGraphicsItem *parent, bool ready)
    : QGraphicsPolygonItem(poly, parent) {
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
  p.setWidthF(Helper::kPointRadius);
  setPen(p);
  setAcceptHoverEvents(true);
}

void PolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget) {
  (void)widget;
    QPen p = pen();
    p.setWidth(Helper::kPointRadius / 2);
    painter->setPen(p);
    const QPolygonF poly = polygon();
    if (m_moveEnable) {
      painter->setBrush(QBrush(Helper::kUnlockedBBoxColor));
    } else {
      painter->setBrush(QBrush(Helper::kLockedBBoxColor));
    }
    painter->drawPolygon(poly);

    if (m_moveEnable) {
        QPen pp = painter->pen();
        int index=0;
        painter->setPen(Qt::NoPen);
        painter->setBrush(pen().color());
        for (auto pt : poly){
            if (m_currentCorner==kNode && index==m_currentNodeIndx_){
                painter->save();
                painter->setPen(p);
                painter->setBrush(Qt::NoBrush);
                painter->drawEllipse(pt, Helper::kPointRadius, Helper::kPointRadius);
                painter->restore();
            } else {
                painter->drawEllipse(pt, Helper::kPointRadius, Helper::kPointRadius);
            }
            ++index;
        }
  }

  if (m_showLabel) {
    painter->setFont(Helper::fontLabel());
    painter->setPen(Qt::black);
    QRectF brect = QGraphicsPolygonItem::boundingRect();
    QRectF lb_rect(brect.center().x(), brect.center().y() - Helper::kLabelRectH,
                   m_labelLen, Helper::kLabelRectH);
    painter->fillRect(lb_rect, Helper::kLabelColor);
    painter->drawText(lb_rect, Qt::AlignVCenter, m_label);
  }
}

void PolygonItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  if (m_currentCorner == kCenter || !m_moveEnable)
    QGraphicsItem::mouseMoveEvent(event);
  else {
    QPointF cpos = event->pos();

    QPolygonF poly = polygon();
    poly[m_currentNodeIndx_] = cpos;
    setPolygon(poly);
    emit dynamic_cast<ImageCanvas *>(scene())->needSaveChanges();
  }
  if (isSelected())
    emit dynamic_cast<ImageCanvas *>(scene())->bboxItemToEditor(this, 0);
}

void PolygonItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
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
      QGraphicsPolygonItem::mousePressEvent(event);
    } else {
      update();
    }
  }
}

void PolygonItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
  QGraphicsPolygonItem::mouseDoubleClickEvent(event);
}

void PolygonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  setCursor(Qt::ArrowCursor);
  QGraphicsPolygonItem::mouseReleaseEvent(event);
  if (isSelected())
    emit dynamic_cast<ImageCanvas *>(scene())->bboxItemToEditor(this, 0);
  m_currentCorner=kInvalid;
  update();
}

PolygonItem::CORNER PolygonItem::positionInsideBBox(const QPointF &pos) {
    const QPolygonF poly = polygon();
    for (int i=0; i<poly.size(); ++i){
        qreal d = Helper::pointLen(poly[i] - pos);
        if (d < Helper::kPointRadius) {
            m_currentNodeIndx_ = i;
            return kNode;
        }
    }
  return kCenter;
}


void PolygonItem::keyPressEvent(QKeyEvent *event) {
  QGraphicsPolygonItem::keyPressEvent(event);
}

void PolygonItem::setLocked(bool what) {
  CustomItem::setLocked(what);
  Helper::setLocked(this, m_moveEnable);
}

QVariant PolygonItem::itemChange(QGraphicsItem::GraphicsItemChange change,
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

QRectF PolygonItem::boundingRect() const {
  QRectF br = QGraphicsPolygonItem::boundingRect();

  const qreal aj = Helper::kPointRadius;
  br = br.adjusted(-aj, -aj, aj, aj);

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

QPainterPath PolygonItem::shape() const
{
    QPainterPath path;
    const QPolygonF poly = polygon();
    path.addPolygon(poly);
    for (const auto &pt : poly){
        path.addEllipse(pt,Helper::kPointRadius/2, Helper::kPointRadius/2);
    }
    QPainterPathStroker spath;
    const QPen p = pen();
    spath.setCapStyle(p.capStyle());
    spath.setJoinStyle(p.joinStyle());
    spath.setWidth(p.widthF());
    spath.setMiterLimit(p.miterLimit());
    QPainterPath op = spath.createStroke(path);
    op.addPath(path);
    return op;
}

void PolygonItem::setLabel(const QString &lb) {
  CustomItem::setLabel(lb);
  setPen(Helper::colorFromLabel(lb));
  auto p = pen();
  p.setWidthF(2 * Helper::kPointRadius);
  setPen(p);
  QFontMetrics fm(Helper::fontLabel());
  m_labelLen = fm.horizontalAdvance(m_label);
}

