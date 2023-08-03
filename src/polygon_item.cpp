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

#include "imagecanvas.h"
#include "undo_cmds.h"

PolygonItem::PolygonItem(const QPolygonF &poly, const QString &label,
                         QGraphicsItem *parent, bool ready, bool closed_poly)
    : QGraphicsPolygonItem(poly, parent) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);

  __setLocked(this, !ready);
  if (ready) {
    setSelected(ready);
  }

  __setLabel(this, label);
  auto p = pen();
  p.setWidthF(Helper::penWidth());
  setPen(p);
  setAcceptHoverEvents(true);
  m_closed_poly = closed_poly;
}

// CustomItem
void PolygonItem::helperParametersChanged() {
  prepareGeometryChange();
  __calculateLabelSize(m_label);
  QPen p = pen();
  p.setWidthF(Helper::penWidth());
  setPen(p);
}

// QGraphicsItem
void PolygonItem::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget *widget) {

  //    auto cl = QColor(Qt::blue);
  //    cl.setAlpha(200);
  //    painter->fillPath(shape(), cl);

  (void)widget;
  QPen p = pen();
  painter->setPen(p);
  const QPolygonF poly = polygon();
  if (m_moveEnable) {
    painter->setBrush(QBrush(Helper::kUnlockedBBoxColor));
  } else {
    painter->setBrush(QBrush(Helper::kLockedBBoxColor));
  }

  if (!m_moveEnable) {
    QPen pp = p;
    pp.setWidth(Helper::kLineWidth);
    pp.setCosmetic(true);
    painter->setPen(pp);
    if (m_closed_poly) {
      painter->drawPolygon(poly);
    } else {
      painter->drawPolyline(poly);
    }
    painter->setPen(p);
  } else {
    painter->save();
    auto pp = p;
    pp.setCosmetic(true);
//    pp.setWidthF(qMin(1.0, p.widthF()));
    pp.setWidthF(Helper::kLineWidth);
    if (m_closed_poly) {
      // pp.setStyle(Qt::DotLine);
      pp.setColor(Qt::black);
      painter->setPen(pp);
      painter->drawPolygon(poly);
      painter->restore();
    } else {
      //      pp.setStyle(Qt::DotLine);
      pp.setColor(Qt::black);
      painter->setPen(pp);
      painter->drawPolyline(poly);
      painter->restore();
    }

    int index = 0;
    painter->setPen(Qt::NoPen);
    QColor color = Helper::getCircleColor(); // pen().color();
    color.setAlpha(150);
    painter->setBrush(color);

    qreal radius = p.widthF();
    for (auto pt : poly) {
      Helper::drawCircleOrSquared(
          painter, pt, radius,
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
  }
}

void PolygonItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  m_currentCorner = kInvalid;
  if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) &&
      event->button() == Qt::LeftButton) {
    __swapStackOrder(this, scene()->items(event->scenePos()));
  } else if (event->modifiers() == Qt::ShiftModifier &&
             event->button() == Qt::LeftButton) {
    setLocked(m_moveEnable);
  } else if (event->button() == Qt::RightButton && m_moveEnable) {
    showEditDialog(this, event->screenPos());
  } else if ((event->modifiers() & (Qt::AltModifier | Qt::MetaModifier)) &&
             event->button() == Qt::LeftButton) {
    auto corner = positionInside(event->pos());
    QPolygonF poly = polygon();
    if (corner == kNode) {
      if (poly.count() > (m_closed_poly ? 3 : 2)) {
        poly.remove(m_currentNodeIndx_);
        Helper::imageCanvas()->undoStack()->push(
            makeChangeCommand(polygon(), poly));
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
            Helper::imageCanvas()->undoStack()->push(
                makeChangeCommand(polygon(), poly));
            break;
          }
        }
      }
    }
  } else {
    m_currentCorner = positionInside(event->pos());
    m_oldPolygon = polygon();
    m_oldPos = pos();
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

  if (m_currentCorner != kInvalid) {
    if (m_oldPos != pos()) {
      Helper::imageCanvas()->undoStack()->push(
          new MoveItemCommand(m_oldPos, pos(), this));
    }

    if (m_oldPolygon != polygon()) {
      Helper::imageCanvas()->undoStack()->push(
          makeChangeCommand(m_oldPolygon, polygon()));
    }
  }
  m_currentCorner = kInvalid;
  update();
}

void PolygonItem::keyPressEvent(QKeyEvent *event) {
  QGraphicsPolygonItem::keyPressEvent(event);
}

QPainterPath PolygonItem::shape() const {
  QPainterPath path;
  const QPolygonF poly = polygon();
  QPen p = pen();

  if (m_closed_poly) {
    path.addPolygon(poly);
  } else {
    path.moveTo(poly[0]);
    for (int i = 1; i < poly.size(); ++i) {
      path.lineTo(poly[i]);
      path.moveTo(poly[i]);
    }
    // path.lineTo(poly[0]);
  }

  for (const auto &pt : poly) {
    path.addEllipse(pt, p.widthF(), p.widthF());
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

// private
PolygonItem::CORNER PolygonItem::positionInside(const QPointF &pos) {
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

QUndoCommand *PolygonItem::makeChangeCommand(const QPolygonF &oldPoly,
                                             const QPolygonF &newPoly) {
  if (m_closed_poly) {
    return new ChangePolygonCommand(oldPoly, newPoly, this, nullptr);
  }
  return new ChangeLineStripCommand(oldPoly, newPoly, this, nullptr);
}

QPolygonF PolygonItem::getPolygonCoords() const {
  return mapToScene(polygon());
}
