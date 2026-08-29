#include "bbox_item.h"

#include <QApplication>
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
#include "undo_cmds.h"
extern Helper globalHelper;

BoundingBoxItem::BoundingBoxItem(ImageCanvas *canvas, const QRectF &rectf,
                                 const QString &label, const QString &dsc,
                                 QGraphicsItem *parent, bool ready)
    : QGraphicsRectItem(rectf, parent) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);

  m_canvas = canvas;
  m_description = dsc;
  //  setPos(rectf.topLeft());
  //  setRect(QRectF(0, 0, rectf.width(), rectf.height()));

  __setLocked(this, !ready);
  if (ready) {
    setSelected(ready);
  }

  __setLabel(this, label);
  auto p = pen();
  p.setWidthF(Helper::penWidth());
  setPen(p);
  // setAcceptHoverEvents(true);
}

void BoundingBoxItem::helperParametersChanged() {
  prepareGeometryChange();
  __calculateLabelSize(m_label);
  QPen p = pen();
  p.setWidthF(Helper::penWidth());
  setPen(p);
}

void BoundingBoxItem::showEditDialog(QGraphicsItem *item,
                                     const QPoint screenPos) {
  EditDialog dlg(static_cast<Helper::CustomItemType>(item->type()), m_label,
                 m_description);
  dlg.setGeometry(QRect{screenPos, dlg.size()});
  dlg.setOccludedTrancatedCrowded(m_occluded, m_truncated, m_crowded);

  if (dlg.exec() == QDialog::Accepted) {
    ImageCanvas *canvas = Helper::imageCanvas();

    if (dlg.removeItem()) {
      emit canvas->deferredRemoveItem(item);
      return;
    }

    if (dlg.label() != m_label) {
      Helper::imageCanvas()->undoStack()->push(
          new ChangeLabelCommand(m_label, dlg.label(), item));
    }

    if (dlg.getOccluded() != m_occluded) {
      Helper::imageCanvas()->undoStack()->push(
          new OccludedChangeBBoxCommand(m_occluded, !m_occluded, this));
    }
    if (dlg.getTruncated() != m_truncated) {
      Helper::imageCanvas()->undoStack()->push(
          new TruncatedChangeBBoxCommand(m_truncated, !m_truncated, this));
    }
    if (dlg.getCrowded() != m_crowded) {
      Helper::imageCanvas()->undoStack()->push(
          new CrowdedChangeBBoxCommand(m_crowded, !m_crowded, this));
    }

    if (dlg.description() != m_description) {
      Helper::imageCanvas()->undoStack()->push(
          new ChangeDescriptionCommand(m_description, dlg.description(), item));
    }
    __setLocked(item, true);
  }
}

void BoundingBoxItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget) {
  (void)widget;
  QPen p = pen();
  painter->setPen(p);

  QRectF brect = rect();  // boundingRect();
  if (m_editEnable) {
    painter->setBrush(QBrush((m_currentCorner == kCenter)
                                 ? Helper::kUnlockedBBoxColorSelected
                                 : Helper::kUnlockedBBoxColor));
  } else {
    painter->setBrush(QBrush(Helper::kLockedBBoxColor));
  }
  if (!m_editEnable) {
    QPen pp = p;
    pp.setWidthF(Helper::kLineWidth);
    pp.setCosmetic(true);
    painter->setPen(pp);
    painter->drawRect(brect);
    painter->setPen(p);
  }
  if (m_editEnable) {
    painter->save();
    auto pp = p;
    pp.setCosmetic(true);
    pp.setWidthF(Helper::kLineWidth);
    pp.setColor(Qt::black);
    painter->setPen(pp);
    painter->drawRect(brect);
    painter->restore();

    painter->setPen(Qt::NoPen);
    QColor color = Helper::getCircleColor(false);
    // color.setAlpha(150);
    painter->setBrush(color);
    qreal w2 = brect.left() + brect.width() / 2;
    qreal h2 = brect.top() + brect.height() / 2;

    qreal w = p.widthF();

    const QPair<QPointF, CORNER> nodes[] = {
        {brect.topLeft(), kTopLeft},
        {{w2, brect.top()}, kTopCenter},
        {brect.topRight(), kTopRight},
        {{brect.right(), h2}, kRightCenter},
        {brect.bottomLeft(), kBottomLeft},
        {{w2, brect.bottom()}, kBottomCenter},
        {brect.bottomRight(), kBottomRight},
        {{brect.left(), h2}, kLeftCenter}};
    for (auto &&[pt, corner] : nodes) {
      if (m_currentCorner == corner) {
        painter->save();
        painter->setBrush(Helper::getCircleColor(true));
        Helper::drawCircleOrSquared(painter, pt, w, m_currentCorner != corner);
        painter->restore();
      } else {
        Helper::drawCircleOrSquared(painter, pt, w, m_currentCorner != corner);
      }
    }
  }
  if (m_canvas->showLabels()) {
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
  if (m_currentCorner == kCenter || !m_editEnable) {
    // A locked item is neither movable nor resizable; the base class ignores
    // the move because ItemIsMovable is cleared.
    QGraphicsRectItem::mouseMoveEvent(event);
  } else {
    QPointF cpos = event->pos();
    QPointF dl = cpos - m_lastPt;

    QRectF newrect = rect();
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
      default:
        break;
    }
    if (newrect.isValid()) {
      // Commit the flipped corner only together with the geometry and the new
      // reference point: updating it while rejecting the rect would leave the
      // corner and m_lastPt out of sync and make the delta accumulate.
      m_currentCorner = new_corner;
      setRect(newrect);
      m_lastPt = cpos;
    }
  }
}

void BoundingBoxItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  m_currentCorner = positionInside(event->pos());
  // Only the press that starts the gesture may set the baseline. A second
  // button pressed mid-drag (e.g. right-click to open the edit dialog while
  // still dragging) would otherwise rebase it to the already-modified geometry
  // and the change in flight would never reach the undo stack.
  if (!m_gestureActive) {
    m_oldCoords = rect();
    m_oldPos = pos();
    m_gestureActive = true;
  }
  // qDebug() << "AA" << m_editEnable << m_currentCorner;
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
      QGraphicsRectItem::mousePressEvent(event);
    } else {
      setCursor(Qt::ArrowCursor);
      m_lastPt = event->pos();
    }
    update();
  }
}

void BoundingBoxItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  setCursor(Qt::ArrowCursor);
  QGraphicsRectItem::mouseReleaseEvent(event);

  // Without an in-flight gesture there is no valid baseline to compare against.
  if (!m_gestureActive) {
    m_currentCorner = kInvalid;
    update();
    return;
  }
  m_gestureActive = false;

  const QRectF newCoords = rect();

  // Position and size are recorded independently: a gesture that changed both
  // must push both commands, otherwise the undo stack cannot restore the
  // original state.
  if (m_oldPos != pos()) {
    Helper::imageCanvas()->undoStack()->push(
        new MoveItemCommand(m_oldPos, pos(), this));
    m_oldPos = pos();
  }
  if (m_oldCoords != newCoords) {
    Helper::imageCanvas()->undoStack()->push(
        new SizeChangeBBoxCommand(m_oldCoords, newCoords, this));
    m_oldCoords = newCoords;
  }
  m_currentCorner = kInvalid;
  update();
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

// set/get
QRectF BoundingBoxItem::boundingBoxCoordinates() {
  return mapRectToScene(rect());
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
  const QRectF brect = rect();  // boundingRect();

  const qreal w2 = brect.left() + brect.width() / 2;
  const qreal h2 = brect.top() + brect.height() / 2;
  const qreal w = pen().widthF();

  const QPair<QPointF, CORNER> nodes[] = {
      {brect.topLeft(), kTopLeft},
      {{w2, brect.top()}, kTopCenter},
      {brect.topRight(), kTopRight},
      {{brect.right(), h2}, kRightCenter},
      {brect.bottomLeft(), kBottomLeft},
      {{w2, brect.bottom()}, kBottomCenter},
      {brect.bottomRight(), kBottomRight},
      {{brect.left(), h2}, kLeftCenter}};

  // Pick the *nearest* handle within the threshold. Testing each handle
  // independently and letting the last match win means that on a box smaller
  // than the handle threshold every test passes and the box can only ever be
  // resized from kLeftCenter, never moved.
  CORNER result = kCenter;
  qreal best = w;
  for (auto &&[pt, corner] : nodes) {
    const qreal d = Helper::pointLen(pt - pos);
    if (d < best) {
      best = d;
      result = corner;
    }
  }
  return result;
}
