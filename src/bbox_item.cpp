#include "bbox_item.h"

#include <QCursor>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>

#include "editdialog.h"
#include "imagecanvas.h"

extern Helper globalHelper;

BoundingBoxItem::BoundingBoxItem(const QRectF &rectf, const QString &label,
                                 QGraphicsItem *parent, bool ready)
    : QGraphicsRectItem(parent) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);
  setPos(rectf.topLeft());
  setRect(QRectF(0, 0, rectf.width(), rectf.height()));

  __setLocked(this, !ready);
  if (ready) {
    setSelected(ready);
  }

  __setLabel(this, label);
  auto p = pen();
  p.setWidthF(Helper::penWidth());
  setPen(p);
  setAcceptHoverEvents(true);
}

void BoundingBoxItem::helperParametersChanged() {
  prepareGeometryChange();
  __calculateLabelSize(m_label);
  QPen p = pen();
  p.setWidthF(Helper::penWidth());
  setPen(p);
}

void BoundingBoxItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget) {
  (void)widget;
  QPen p = pen();
  painter->setPen(p);

  QRectF brect = rect();  // boundingRect();
  if (m_moveEnable) {
    painter->setBrush(QBrush(Helper::kUnlockedBBoxColor));
  } else {
    painter->setBrush(QBrush(Helper::kLockedBBoxColor));
  }
  if (!m_moveEnable) {
    QPen pp = p;
    pp.setWidthF(Helper::kLineWidth);
    pp.setCosmetic(true);
    painter->setPen(pp);
    painter->drawRect(brect);
    painter->setPen(p);
  }
  if (m_moveEnable) {
    painter->save();
    auto pp = p;
    pp.setWidthF(qMin(1.0, p.widthF()));
    pp.setStyle(Qt::DotLine);
    pp.setColor(Qt::black);
    painter->setPen(pp);
    painter->drawRect(brect);
    painter->restore();

    painter->setPen(Qt::NoPen);
    QColor color = pen().color();
    color.setAlpha(150);
    painter->setBrush(color);
    qreal w2 = brect.left() + brect.width() / 2;
    qreal h2 = brect.top() + brect.height() / 2;

    qreal w = p.widthF();
    Helper::drawCircleOrSquared(painter, brect.topLeft(), w,
                                m_currentCorner != kTopLeft);
    Helper::drawCircleOrSquared(painter, {w2, brect.top()}, w,
                                m_currentCorner != kTopCenter);
    Helper::drawCircleOrSquared(painter, brect.topRight(), w,
                                m_currentCorner != kTopRight);
    Helper::drawCircleOrSquared(painter, {brect.right(), h2}, w,
                                m_currentCorner != kRightCenter);
    Helper::drawCircleOrSquared(painter, brect.bottomLeft(), w,
                                m_currentCorner != kBottomLeft);
    Helper::drawCircleOrSquared(painter, {w2, brect.bottom()}, w,
                                m_currentCorner != kBottomCenter);
    Helper::drawCircleOrSquared(painter, brect.bottomRight(), w,
                                m_currentCorner != kBottomRight);
    Helper::drawCircleOrSquared(painter, {brect.left(), h2}, w,
                                m_currentCorner != kLeftCenter);
  }
  if (m_showLabel) {
    painter->setFont(globalHelper.fontLabel());
    p.setColor(Qt::black);
    painter->setPen(p);

    brect = boundingRect();
    QRectF lb_rect(brect.x(), brect.y(), m_labelLen, m_labelHeight);
    painter->fillRect(lb_rect, Helper::kLabelColor);
    painter->drawText(lb_rect, Qt::AlignVCenter | Qt::AlignHCenter, m_label);
  }
}

void BoundingBoxItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  if (m_currentCorner == kCenter || !m_moveEnable)
    QGraphicsRectItem::mouseMoveEvent(event);
  else {
    QPointF cpos = mapToScene(event->pos());
    QPointF dl = cpos - m_lastPt;

    QRectF srect = scene()->sceneRect();
    QRectF newrect(this->pos(), this->rect().size());

    CORNER new_corner = m_currentCorner;
    bool sw;
    bool sh;

    switch (m_currentCorner) {
      case kTopLeft:
        newrect = buildRectFromTwoPoints(newrect.topLeft() + dl,
                                         newrect.bottomRight(), sw, sh);
        if (sw && sh)
          new_corner = kBottomRight;
        else if (sw)
          new_corner = kTopRight;
        else if (sh)
          new_corner = kBottomLeft;
        break;
      case kTopRight:
        newrect = buildRectFromTwoPoints(newrect.topRight() + dl,
                                         newrect.bottomLeft(), sw, sh);
        if (!sw && sh)
          new_corner = kBottomLeft;
        else if (!sw)
          new_corner = kTopLeft;
        else if (sh)
          new_corner = kBottomRight;
        break;
      case kBottomRight:
        newrect = buildRectFromTwoPoints(newrect.bottomRight() + dl,
                                         newrect.topLeft(), sw, sh);
        if (!sw && !sh)
          new_corner = kTopLeft;
        else if (!sw)
          new_corner = kBottomLeft;
        else if (!sh)
          new_corner = kTopRight;
        break;
      case kBottomLeft:
        newrect = buildRectFromTwoPoints(newrect.bottomLeft() + dl,
                                         newrect.topRight(), sw, sh);
        if (sw && !sh)
          new_corner = kTopRight;
        else if (sw)
          new_corner = kBottomRight;
        else if (!sh)
          new_corner = kTopLeft;
        break;
      case kTopCenter:
        dl.setX(0);
        newrect = buildRectFromTwoPoints(newrect.topLeft() + dl,
                                         newrect.bottomRight(), sw, sh);
        if (sh) new_corner = kBottomCenter;
        break;
      case kBottomCenter:
        dl.setX(0);
        newrect = buildRectFromTwoPoints(newrect.bottomRight() + dl,
                                         newrect.topLeft(), sw, sh);
        if (!sh) new_corner = kTopCenter;
        break;
      case kRightCenter:
        dl.setY(0);
        newrect = buildRectFromTwoPoints(newrect.topRight() + dl,
                                         newrect.bottomLeft(), sw, sh);
        if (!sw) new_corner = kLeftCenter;
        break;
      case kLeftCenter:
        dl.setY(0);
        newrect = buildRectFromTwoPoints(newrect.topLeft() + dl,
                                         newrect.bottomRight(), sw, sh);
        if (sw) new_corner = kRightCenter;
        break;
    }

    m_currentCorner = new_corner;
    newrect &= srect;
    if (newrect.isValid()) {
      setPos(newrect.topLeft());
      setRect(0, 0, newrect.width(), newrect.height());
      m_lastPt = cpos;
      emit dynamic_cast<ImageCanvas *>(scene())->needSaveChanges();
    }
  }
  if (isSelected())
    emit dynamic_cast<ImageCanvas *>(scene())->bboxItemToEditor(this, 0);
}

void BoundingBoxItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) &&
      event->button() == Qt::LeftButton) {
    __swapStackOrder(this, scene()->items(event->scenePos()));
  } else if (event->modifiers() == Qt::ShiftModifier &&
             event->button() == Qt::LeftButton) {
    setLocked(m_moveEnable);
  } else if (event->button() == Qt::RightButton && m_moveEnable) {
    __showEditDialog(this, event->screenPos());
  } else {
    m_currentCorner = positionInside(event->pos());
    if (m_currentCorner == kCenter || !m_moveEnable) {
      QGraphicsRectItem::mousePressEvent(event);
    } else {
      update();
      m_lastPt = mapToScene(event->pos());
    }
  }
}

void BoundingBoxItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
  QGraphicsRectItem::mouseDoubleClickEvent(event);
}

void BoundingBoxItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  setCursor(Qt::ArrowCursor);
  QGraphicsRectItem::mouseReleaseEvent(event);
  if (isSelected())
    emit dynamic_cast<ImageCanvas *>(scene())->bboxItemToEditor(this, 0);
  m_currentCorner = kInvalid;
  update();
}

void BoundingBoxItem::keyPressEvent(QKeyEvent *event) {
  if (m_moveEnable &&
      (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
       event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)) {
    QRectF newrect(this->pos(), this->rect().size());
    double dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    double sx = 1, sy = 1;

    if (event->modifiers() & Qt::ShiftModifier) {
      sx = std::max(newrect.width() * 0.1, 1.0);
      sy = std::max(newrect.height() * 0.1, 1.0);
    }

    if (event->modifiers() & Qt::ControlModifier) {
      switch (event->key()) {
        case Qt::Key_Left:
          dx2 = -sx;
          break;
        case Qt::Key_Right:
          dx2 = +sx;
          break;
        case Qt::Key_Up:
          dy2 = -sy;
          break;
        case Qt::Key_Down:
          dy2 = sy;
          break;
      }
    } else {
      switch (event->key()) {
        case Qt::Key_Left:
          dx1 = -sx;
          break;
        case Qt::Key_Right:
          dx1 = +sx;
          break;
        case Qt::Key_Up:
          dy1 = -sy;
          break;
        case Qt::Key_Down:
          dy1 = sy;
          break;
      }
    }
    newrect.adjust(dx1, dy1, dx2, dy2);
    newrect &= scene()->sceneRect();
    if (newrect.isValid()) {
      setPos(newrect.topLeft());
      setRect(0, 0, newrect.width(), newrect.height());
    }
  } else if (event->key() == Qt::Key_Return) {
    this->setLocked(true);
    // setCursor(Qt::ArrowCursor);
  } else
    QGraphicsItem::keyPressEvent(event);
  if (isSelected())
    emit dynamic_cast<ImageCanvas *>(scene())->bboxItemToEditor(this, 0);
}

QPainterPath BoundingBoxItem::shape() const {
  QPainterPath path;
  const QRectF brect = rect();
  qreal w2 = brect.left() + brect.width() / 2;
  qreal h2 = brect.top() + brect.height() / 2;
  path.addRect(brect);

  qreal w = pen().widthF() / 2.0;
  path.addEllipse(brect.topLeft(), w, w);
  path.addEllipse({w2, brect.top()}, w, w);

  path.addEllipse(brect.topRight(), w, w);
  path.addEllipse({brect.right(), h2}, w, w);

  path.addEllipse(brect.bottomLeft(), w, w);
  path.addEllipse({w2, brect.bottom()}, w, w);

  path.addEllipse(brect.bottomRight(), w, w);
  path.addEllipse({brect.left(), h2}, w, w);

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

QRectF BoundingBoxItem::boundingRect() const {
  QRectF br = QGraphicsRectItem::boundingRect();
  const qreal aj = pen().widthF() / 2;
  br = br.adjusted(-aj, -aj, aj, aj);

  double dw = 0;
  if (rect().width() < m_labelLen) {
    dw = m_labelLen - rect().width();
  }
  return br.adjusted(0, -m_labelHeight, dw, 0);
}

QVariant BoundingBoxItem::itemChange(QGraphicsItem::GraphicsItemChange change,
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

// set/get
QRectF BoundingBoxItem::boundingBoxCoordinates() {
  return mapRectToScene(rect());
}

void BoundingBoxItem::setCoordinates(const QRectF &coords) {
  setPos(coords.topLeft());
  setRect(QRectF(QPointF(0, 0), coords.size()));
}

// private
QRectF BoundingBoxItem::buildRectFromTwoPoints(const QPointF &p1,
                                               const QPointF &p2, bool &sw,
                                               bool &sh) {
  sw = p2.x() < p1.x();
  sh = p2.y() < p1.y();
  double rw = qAbs(p2.x() - p1.x());
  double rh = qAbs(p2.y() - p1.y());
  QPointF topleft(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y()));
  return {topleft, QSizeF(rw, rh)};
}

BoundingBoxItem::CORNER BoundingBoxItem::positionInside(const QPointF &pos) {
  CORNER result = kCenter;
  const QRectF brect = rect();  // boundingRect();

  qreal w2 = brect.left() + brect.width() / 2;
  qreal h2 = brect.top() + brect.height() / 2;
  qreal w = pen().widthF();

  if (Helper::pointLen(brect.topLeft() - pos) < w) {
    result = kTopLeft;
  }
  if (Helper::pointLen(QPointF{w2, brect.top()} - pos) < w) {
    result = kTopCenter;
  }

  if (Helper::pointLen(brect.topRight() - pos) < w) {
    result = kTopRight;
  }

  if (Helper::pointLen(QPointF{brect.right(), h2} - pos) < w) {
    result = kRightCenter;
  }

  if (Helper::pointLen(brect.bottomLeft() - pos) < w) {
    result = kBottomLeft;
  }

  if (Helper::pointLen(QPointF{w2, brect.bottom()} - pos) < w) {
    result = kBottomCenter;
  }

  if (Helper::pointLen(brect.bottomRight() - pos) < w) {
    result = kBottomRight;
  }

  if (Helper::pointLen(QPointF{brect.left(), h2} - pos) < w) {
    result = kLeftCenter;
  }
  return result;
}
