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
  p.setWidthF(Helper::kPointRadius);
  setPen(p);
  setAcceptHoverEvents(true);
}

void PolygonItem::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
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
    int index = 0;
    painter->setPen(Qt::NoPen);
    painter->setBrush(pen().color());
    for (auto pt : poly) {
      if (m_currentCorner == kNode && index == m_currentNodeIndx_) {
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
    QRectF bb = boundingRect();
    QPointF bc = bb.center();
    qreal dw = m_labelLen / 2.0;
    qreal dh = Helper::kLabelRectH / 2.0;
    QRectF brect(bc - QPointF{dw, dh}, QSizeF(m_labelLen, Helper::kLabelRectH));
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
  for (int i = 0; i < poly.size(); ++i) {
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

void PolygonItem::setLocked(bool what) { __setLocked(this, what); }

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
  if (br.width() < m_labelLen) {
    dw = (m_labelLen - br.width()) / 2.0;
  }
  if (br.height() < Helper::kLabelRectH) {
    dh = (Helper::kLabelRectH - br.height()) / 2.0;
  }
  return br.adjusted(-dw, -dh, dw, dh);
}

QPainterPath PolygonItem::shape() const {
  QPainterPath path;
  const QPolygonF poly = polygon();
  path.addPolygon(poly);
  for (const auto &pt : poly) {
    path.addEllipse(pt, Helper::kPointRadius / 2, Helper::kPointRadius / 2);
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
