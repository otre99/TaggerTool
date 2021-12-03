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

PolygonItem::PolygonItem(const QPolygonF &poly, const QString &label,
                         QGraphicsItem *parent, bool ready)
    : QGraphicsPolygonItem(poly, parent) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);

  __setLocked(this, !ready);
  if (ready) {
    setSelected(ready);
  }

  __setLabel(this, label);
  auto p = pen();
  p.setWidthF(Helper::kPointRadius * Helper::kInvScaleFactor);
  setPen(p);
  setAcceptHoverEvents(true);
}

void PolygonItem::helperParametersChanged() {
  prepareGeometryChange();
  __calculateLabelSize(m_label);
  QPen p = pen();
  p.setWidthF(Helper::kPointRadius * Helper::kInvScaleFactor);
  setPen(p);
}

void PolygonItem::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget *widget) {
  (void)widget;
  QPen p = pen();
  painter->setPen(p);
  const QPolygonF poly = polygon();
  if (m_moveEnable) {
    painter->setBrush(QBrush(Helper::kUnlockedBBoxColor));
  } else {
    painter->setBrush(QBrush(Helper::kLockedBBoxColor));
  }

  if (!m_moveEnable)
    painter->drawPolygon(poly);

  if (m_moveEnable) {
    painter->save();
    auto pp = p;
    pp.setWidth(1);
    pp.setStyle(Qt::DotLine);
    pp.setColor(Qt::black);
    painter->setPen(pp);
    painter->drawPolygon(poly);
    painter->restore();

    int index = 0;
    painter->setPen(Qt::NoPen);
    QColor color = pen().color();
    color.setAlpha(150);
    painter->setBrush(color);

    qreal w = p.widthF();
    for (auto pt : poly) {
      Helper::drawCircleOrSquared(
          painter, pt, w,
          (m_currentCorner != kNode || index != m_currentNodeIndx_));
      ++index;
    }
  }

  if (m_showLabel) {
    painter->setFont(Helper::fontLabel());
    painter->setPen(Qt::black);
    QRectF bb = boundingRect();
    QPointF bc = bb.center();
    qreal dw = m_labelLen / 2.0;
    qreal dh = m_labelHeight / 2.0;
    QRectF brect(bc - QPointF{dw, dh}, QSizeF(m_labelLen, m_labelHeight));
    painter->fillRect(brect, Helper::kLabelColor);
    painter->drawText(brect, Qt::AlignVCenter | Qt::AlignHCenter, m_label);
  }
}

void PolygonItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  if (m_currentCorner == kCenter || !m_moveEnable)
    QGraphicsItem::mouseMoveEvent(event);
  else if (m_currentCorner == kNode) {
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
  } else if (event->modifiers() == Qt::AltModifier &&
             event->button() == Qt::LeftButton) {
    auto corner = positionInsideBBox(event->pos());
    QPolygonF poly = polygon();
    if (corner == kNode) {
      if (poly.count() > 3) {
        poly.remove(m_currentNodeIndx_);
        setPolygon(poly);
      }
    } else {
      qreal th = pen().widthF();
      for (int i = 0; i < poly.count(); ++i) {
        const QPointF p1 = poly[i];
        const QPointF p2 = i == poly.count() - 1 ? poly[0] : poly[i + 1];
        const double dist =
            Helper::distanceToLine(QVector2D{p1}, QVector2D{p2}, event->pos());
        if (dist < th) {
          QPointF newPt = Helper::pointLineIntersection(
              QVector2D{p1}, QVector2D{p2}, event->pos());
          QPointF midPt = 0.5 * (p1 + p2);
          const double len = Helper::pointLen(p1 - p2);
          if (Helper::pointLen(newPt - midPt) <= len / 2.0) {
            poly.insert(i + 1, newPt);
            setPolygon(poly);
            break;
          }
        }
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
  m_currentCorner = kInvalid;
  update();
}

PolygonItem::CORNER PolygonItem::positionInsideBBox(const QPointF &pos) {
  const QPolygonF poly = polygon();
  qreal th = pen().widthF();
  for (int i = 0; i < poly.size(); ++i) {
    qreal d = Helper::pointLen(poly[i] - pos);
    if (d < th) {
      m_currentNodeIndx_ = i;
      return kNode;
    }
  }
  return kCenter;
}

void PolygonItem::keyPressEvent(QKeyEvent *event) {
  QGraphicsPolygonItem::keyPressEvent(event);
}

void PolygonItem::setLocked(bool what) { __setLocked(this, what); }

QVariant PolygonItem::itemChange(QGraphicsItem::GraphicsItemChange change,
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

QRectF PolygonItem::boundingRect() const {
  QRectF br = QGraphicsPolygonItem::boundingRect();
  qreal o = pen().widthF();
  br = br.adjusted(-o, -o, o, o);

  double dw = 0;
  double dh = 0;
  if (br.width() < m_labelLen) {
    dw = (m_labelLen - br.width()) / 2.0;
  }
  if (br.height() < m_labelHeight) {
    dh = (m_labelHeight - br.height()) / 2.0;
  }
  return br.adjusted(-dw, -dh, dw, dh);
}

QPainterPath PolygonItem::shape() const {
  QPainterPath path;
  const QPolygonF poly = polygon();
  const QPen p = pen();
  path.addPolygon(poly);
  for (const auto &pt : poly) {
    path.addEllipse(pt, p.widthF() / 2, p.widthF() / 2);
  }
  QPainterPathStroker spath;
  spath.setCapStyle(p.capStyle());
  spath.setJoinStyle(p.joinStyle());
  spath.setWidth(p.widthF());
  spath.setMiterLimit(p.miterLimit());
  QPainterPath op = spath.createStroke(path);
  op.addPath(path);
  return op;
}
