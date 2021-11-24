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
    : QGraphicsRectItem(parent), m_showCorners(ready) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);
  setPos(rectf.topLeft());
  setRect(QRectF(0, 0, rectf.width(), rectf.height()));

  __setLocked(this, !ready);
  if (ready) {
    setSelected(ready);
  }

  m_label = label;
  setPen(Helper::colorFromLabel(m_label));
  QFontMetrics fm(globalHelper.fontLabel());
  m_labelLen = fm.horizontalAdvance("  " + m_label) + Helper::kLabelRectH;
  setAcceptHoverEvents(true);
}

QRectF BoundingBoxItem::boundingBoxCoordinates() {
  const QSizeF sf = rect().size();
  const QPointF tl = pos();
  return {tl.x(), tl.y(), sf.width(), sf.height()};
}

QRectF BoundingBoxItem::boundingRect() const {
  double dw = 0.0;
  if (rect().width() < m_labelLen) {
    dw = m_labelLen - rect().width();
  }

  return rect().adjusted(-0.5, -Helper::kLabelRectH, dw + 0.5, 0.5);
}

void BoundingBoxItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget) {
  (void)widget;
  painter->setFont(globalHelper.fontLabel());
  painter->setPen(Qt::black);
  QRectF brect = boundingRect();

  if (m_showLabel) {
    QColor markerColor = pen().color();

    auto r =
        QRectF{brect.x(), brect.y(), static_cast<qreal>(Helper::kLabelRectH),
               static_cast<qreal>(Helper::kLabelRectH)};
    if (m_moveEnable) {
      painter->setBrush(markerColor);
      painter->drawEllipse(r.adjusted(0.5, 0.5, -0.5, -0.5));
    } else {
      painter->fillRect(
          QRectF{brect.x(), brect.y(), static_cast<qreal>(Helper::kLabelRectH),
                 static_cast<qreal>(Helper::kLabelRectH)},
          markerColor);
    }

    QRectF lb_rect(brect.x() + Helper::kLabelRectH, brect.y(),
                   m_labelLen - Helper::kLabelRectH, Helper::kLabelRectH);
    painter->fillRect(lb_rect, Helper::kLabelColor);
    painter->drawText(lb_rect, Qt::AlignVCenter, " " + m_label);
  }

  brect = rect();  // boundingRect();
  painter->setPen(pen());
  if (m_moveEnable) {
    painter->setBrush(QBrush(Helper::kUnlockedBBoxColor));
  } else {
    painter->setBrush(QBrush(Helper::kLockedBBoxColor));
  }
  painter->drawRect(brect);
  if (m_moveEnable) {
    const double ww = brect.width();
    const double hh = brect.height();
    m_marginW =
        qMin(Helper::kBorderSize,
             ww * 0.25);  // qMax(kBorderSize/4, qMin(kBorderSize, ww * 0.1));
    m_marginH =
        qMin(Helper::kBorderSize,
             hh * 0.25);  // qMax(kBorderSize/4, qMin(kBorderSize, hh * 0.1));
    m_marginW = m_marginH = qMin(m_marginW, m_marginH);

    if (qMin(ww, hh) > 0.0) {
      painter->setPen(Qt::NoPen);
      painter->setBrush(QBrush(Helper::kMarginBBoxColor));

      switch (m_currentCorner) {
        case kTopLeft:
          painter->drawRect(
              QRectF(brect.x() + 0.5, brect.y() + 0.5, m_marginW, m_marginH));
          break;
        case kTopCenter:
          painter->drawRect(
              QRectF(QPointF(brect.x() + m_marginW + 0.5, brect.y() + 0.5),
                     QPointF(brect.x() + brect.width() - m_marginW - 0.5,
                             brect.y() + 0.5 + m_marginH)));
          break;
        case kTopRight:
          painter->drawRect(QRectF(brect.x() + brect.width() - m_marginW - 0.5,
                                   brect.y() + 0.5, m_marginW, m_marginH));
          break;
        case kRightCenter:
          painter->drawRect(
              QRectF(QPointF(brect.x() + brect.width() - m_marginW - 0.5,
                             brect.y() + 0.5 + m_marginH),
                     QPointF(brect.x() + brect.width() - 0.5,
                             brect.y() + brect.height() - 0.5 - m_marginH)));
          break;
        case kBottomRight:
          painter->drawRect(QRectF(brect.x() + brect.width() - m_marginW - 0.5,
                                   brect.y() + brect.height() - 0.5 - m_marginH,
                                   m_marginW, m_marginH));
          break;
        case kBottomCenter:
          painter->drawRect(
              QRectF(QPointF(brect.x() + m_marginW + 0.5,
                             brect.y() + brect.height() - 0.5 - m_marginH),
                     QPointF(brect.x() + brect.width() - 0.5 - m_marginW,
                             brect.y() + brect.height() - 0.5)));
          break;
        case kBottomLeft:
          painter->drawRect(QRectF(brect.x() + 0.5,
                                   brect.y() + brect.height() - 0.5 - m_marginH,
                                   m_marginW, m_marginH));
          break;
        case kLeftCenter:
          painter->drawRect(
              QRectF(QPointF(brect.x() + 0.5, brect.y() + 0.5 + m_marginH),
                     QPointF(brect.x() + m_marginW + 0.5,
                             brect.y() + brect.height() - 0.5 - m_marginH)));
          break;
      }
    }
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
      // prepareGeometryChange();
      setPos(newrect.topLeft());
      setRect(0, 0, newrect.width(), newrect.height());
      m_lastPt = cpos;
      emit dynamic_cast<ImageCanvas *>(scene())->needSaveChanges();
    }
  }
  if (isSelected())
    emit dynamic_cast<ImageCanvas *>(scene())->bboxItemToEditor(this, 0);
}

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

void BoundingBoxItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
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

BoundingBoxItem::CORNER BoundingBoxItem::positionInsideBBox(
    const QPointF &pos) {
  CORNER result = kCenter;
  const QRectF brect = rect();  // boundingRect();

  if (QRectF(brect.x() + 0.5, brect.y() + 0.5, m_marginW, m_marginH)
          .contains(pos)) {
    result = kTopLeft;
    if (m_moveEnable) setCursor(Qt::SizeFDiagCursor);
  } else if (QRectF(brect.x() + brect.width() - m_marginW - 0.5,
                    brect.y() + 0.5, m_marginW, m_marginH)
                 .contains(pos)) {
    result = kTopRight;
    if (m_moveEnable) setCursor(Qt::SizeBDiagCursor);
  } else if (QRectF(brect.x() + brect.width() - m_marginW - 0.5,
                    brect.y() + brect.height() - 0.5 - m_marginH, m_marginW,
                    m_marginH)
                 .contains(pos)) {
    result = kBottomRight;
    if (m_moveEnable) setCursor(Qt::SizeFDiagCursor);
  } else if (QRectF(brect.x() + 0.5,
                    brect.y() + brect.height() - 0.5 - m_marginH, m_marginW,
                    m_marginH)
                 .contains(pos)) {
    result = kBottomLeft;
    if (m_moveEnable) setCursor(Qt::SizeBDiagCursor);
  } else if (QRectF(QPointF(brect.x() + m_marginW + 0.5, brect.y() + 0.5),
                    QPointF(brect.x() + brect.width() - m_marginW - 0.5,
                            brect.y() + 0.5 + m_marginH))
                 .contains(pos)) {
    result = kTopCenter;
    if (m_moveEnable) setCursor(Qt::SizeVerCursor);
  } else if (QRectF(QPointF(brect.x() + brect.width() - m_marginW - 0.5,
                            brect.y() + 0.5 + m_marginH),
                    QPointF(brect.x() + brect.width() - 0.5,
                            brect.y() + brect.height() - 0.5 - m_marginH))
                 .contains(pos)) {
    result = kRightCenter;
    if (m_moveEnable) setCursor(Qt::SizeHorCursor);
  } else if (QRectF(QPointF(brect.x() + m_marginW + 0.5,
                            brect.y() + brect.height() - 0.5 - m_marginH),
                    QPointF(brect.x() + brect.width() - 0.5 - m_marginW,
                            brect.y() + brect.height() - 0.5))
                 .contains(pos)) {
    result = kBottomCenter;
    if (m_moveEnable) setCursor(Qt::SizeVerCursor);
  } else if (QRectF(QPointF(brect.x() + 0.5, brect.y() + 0.5 + m_marginH),
                    QPointF(brect.x() + m_marginW + 0.5,
                            brect.y() + brect.height() - 0.5 - m_marginH))
                 .contains(pos)) {
    result = kLeftCenter;
    if (m_moveEnable) setCursor(Qt::SizeHorCursor);
  } else {
    if (m_moveEnable) setCursor(Qt::SizeAllCursor);
  }
  return result;
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

void BoundingBoxItem::setLocked(bool what) { __setLocked(this, what); }

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

void BoundingBoxItem::setLabel(const QString &lb) {
  __setLabel(this, lb);
  m_labelLen += Helper::kLabelRectH;
}

void BoundingBoxItem::setCoordinates(const QRectF &coords) {
  setPos(coords.topLeft());
  setRect(QRectF(QPointF(0, 0), coords.size()));
}
