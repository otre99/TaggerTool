#include "bboxitem.h"
#include "imagecanvas.h"
#include "utils.h"
#include <QCursor>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>

namespace {
const QColor kUnlockedBBoxColor(0, 0, 128, 64);
const QColor kLockedBBoxColor(128, 128, 128, 64);
const QColor kMarginBBoxColor(128, 0, 0, 64);
const QColor kLabelColor(200, 200, 200, 64);
const int kLabelRectSize = 12;
} // namespace

BoundingBoxItem::BoundingBoxItem(const QRectF &rectf, const QString &label,
                                 QGraphicsItem *parent, bool ready)
    : QGraphicsRectItem(parent), m_showCorners(ready), m_moveEnable(ready) {
  setFlags(QGraphicsItem::ItemIsFocusable |
           QGraphicsItem::ItemSendsGeometryChanges);
  setPos(rectf.topLeft());
  setRect(QRectF(0, 0, rectf.width(), rectf.height()));

  setLocked(!ready);
  if (ready) {
    setSelected(ready);
  }
  setLabel(label);
  setAcceptHoverEvents(true);
}

QRectF BoundingBoxItem::boundingBoxCoordinates() {
  const QSizeF sf = rect().size();
  const QPointF tl = pos();
  return {tl.x(), tl.y(), sf.width(), sf.height()};
}

QRectF BoundingBoxItem::boundingRect() const {
  const QRectF r = QGraphicsRectItem::boundingRect();
  return r;
}

void BoundingBoxItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget) {
  QGraphicsRectItem::paint(painter, option, widget);
  const QRectF brect = boundingRect();

  if (!m_moveEnable || !isSelected()) {
    QRectF lb_rect;
    lb_rect.setTopLeft(brect.topLeft());
    lb_rect.setWidth(brect.width());
    lb_rect.setHeight(kLabelRectSize);
    painter->fillRect(lb_rect, kLabelColor);

    QFont font = painter->font();
    font.setPixelSize(11);
    painter->setFont(font);
    painter->drawText(lb_rect, Qt::AlignCenter, m_info);
  }

  if (m_moveEnable) {
    const double ww = brect.width();
    const double hh = brect.height();
    m_marginW = qMax(10.0, qMin(50.0, ww * 0.1));
    m_marginH = qMax(10.0, qMin(50.0, hh * 0.1));
    m_marginW = m_marginH = qMin(m_marginW, m_marginH);

    if (qMin(ww, hh) > 10) {
      painter->setPen(Qt::NoPen);
      painter->setBrush(QBrush(kMarginBBoxColor));

      switch (m_currentCorner) {
      case kTopLeft:
        painter->drawRect(
            QRectF(brect.x() + 0.5, brect.y() + 0.5, m_marginW, m_marginH));
        break;
      case kTopCenter:
        painter->drawRect(
            QRectF(QPointF(brect.x() + m_marginW - 0.5, brect.y() + 0.5),
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
    qDebug() << "AAAAAAAAAAAAAAAAAAAAAAAA" << boundingBoxCoordinates();
  }
  qDebug() << "BBBBBBBBBBBBBBBBBBBB" << boundingBoxCoordinates();
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
      if (sh)
        new_corner = kBottomCenter;
      break;
    case kBottomCenter:
      dl.setX(0);
      newrect = buildRectFromTwoPoints(newrect.bottomRight() + dl,
                                       newrect.topLeft(), sw, sh);
      if (!sh)
        new_corner = kTopCenter;
      break;
    case kRightCenter:
      dl.setY(0);
      newrect = buildRectFromTwoPoints(newrect.topRight() + dl,
                                       newrect.bottomLeft(), sw, sh);
      if (!sw)
        new_corner = kLeftCenter;
      break;
    case kLeftCenter:
      dl.setY(0);
      newrect = buildRectFromTwoPoints(newrect.topLeft() + dl,
                                       newrect.bottomRight(), sw, sh);
      if (sw)
        new_corner = kRightCenter;
      break;
    }

    m_currentCorner = new_corner;
    newrect &= srect;
    if (newrect.isValid()) {
      prepareGeometryChange();
      setPos(newrect.topLeft());
      setRect(0, 0, newrect.width(), newrect.height());
      m_lastPt = cpos;
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
    QPointF pt = mapToScene(event->pos());
    const auto l = scene()->items(pt);
    if (l.count() > 1) {
      for (int i = 1; i < l.count(); ++i)
        this->stackBefore(l[i]);
      auto *bbox = dynamic_cast<BoundingBoxItem *>(l[1]);
      if (bbox) {
        bbox->setLocked(false);
        this->setLocked(true);
      }
    }
  } else if (event->modifiers() == Qt::ShiftModifier &&
             event->button() == Qt::LeftButton) {
    setLocked(m_moveEnable);
  } else if (event->button() == Qt::RightButton && m_moveEnable) {
    QMenu menu;
    QAction *editLabel = menu.addAction("Edit label");
    QAction *selectedAction = menu.exec(event->screenPos());
    if (selectedAction == editLabel) {
      QInputDialog dlg;
      dlg.setTextValue("unknow");
      dlg.move(event->screenPos());
      if (dlg.exec() == QDialog::Accepted) {
        m_info = dlg.textValue();
        update();
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
}

BoundingBoxItem::CORNER
BoundingBoxItem::positionInsideBBox(const QPointF &pos) {
  const double x = pos.x();
  const double y = pos.y();
  CORNER result = kCenter;
  const QRectF brect = boundingRect();
  if ((brect.x() <= x) && (x <= brect.x() + m_marginW) && (brect.y() <= y) &&
      (y <= brect.y() + m_marginH)) {
    result = kTopLeft;
    if (m_moveEnable)
      setCursor(Qt::SizeFDiagCursor);
  } else if ((brect.x() + brect.width() - m_marginW <= x) &&
             (x <= brect.x() + brect.width()) && (brect.y() <= y) &&
             (y <= brect.y() + m_marginH)) {
    result = kTopRight;
    if (m_moveEnable)
      setCursor(Qt::SizeBDiagCursor);
  } else if ((brect.x() + brect.width() - m_marginW <= x) &&
             (x <= brect.x() + brect.width()) &&
             (brect.y() + brect.height() - m_marginH <= y) &&
             (y <= brect.y() + brect.height())) {
    result = kBottomRight;
    if (m_moveEnable)
      setCursor(Qt::SizeFDiagCursor);
  } else if ((brect.x() <= x) && (x <= brect.x() + m_marginW) &&
             (brect.y() + brect.height() - m_marginH <= y) &&
             (y <= brect.y() + brect.height())) {
    result = kBottomLeft;
    if (m_moveEnable)
      setCursor(Qt::SizeBDiagCursor);
  } else if ((brect.x() + m_marginW <= x) &&
             (x <= brect.x() + brect.width() - m_marginW) && (brect.y() <= y) &&
             (y <= brect.y() + m_marginH)) {
    result = kTopCenter;
    if (m_moveEnable)
      setCursor(Qt::SizeVerCursor);
  } else if ((brect.x() + brect.width() - m_marginW <= x) &&
             (x <= brect.x() + brect.width()) && (brect.y() + m_marginH <= y) &&
             (y <= brect.y() + brect.height() - m_marginH)) {
    result = kRightCenter;
    if (m_moveEnable)
      setCursor(Qt::SizeHorCursor);
  } else if ((brect.x() + m_marginW <= x) &&
             (x <= brect.x() + brect.width() - m_marginW) &&
             (brect.y() + brect.height() - m_marginH <= y) &&
             (y <= brect.y() + brect.height())) {
    result = kBottomCenter;
    if (m_moveEnable)
      setCursor(Qt::SizeVerCursor);
  } else if ((brect.x() <= x) && (x <= brect.x() + m_marginW) &&
             (brect.y() + m_marginH <= y) &&
             (y <= brect.y() + brect.height() - m_marginH)) {
    result = kLeftCenter;
    if (m_moveEnable)
      setCursor(Qt::SizeHorCursor);
  } else {
    if (m_moveEnable)
      setCursor(Qt::SizeAllCursor);
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
      prepareGeometryChange();
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

void BoundingBoxItem::setLocked(bool what) {
  m_moveEnable = !what;
  setFlag(QGraphicsItem::ItemIsMovable, m_moveEnable);
  setFlag(QGraphicsItem::ItemIsSelectable, m_moveEnable);
  setFlag(QGraphicsItem::ItemIsFocusable, m_moveEnable);
  setBrush(m_moveEnable ? kUnlockedBBoxColor : kLockedBBoxColor);
}

QVariant BoundingBoxItem::itemChange(QGraphicsItem::GraphicsItemChange change,
                                     const QVariant &value) {
  // TODO (rbt): I decided reimplement this function for some reason, but  I
  // don't remember well!!!!!

  return value;
}

void BoundingBoxItem::setLabel(const QString &lb) {
  m_info = lb;
  setPen(Helper::colorFromLabel(lb));
  setToolTip(m_info);
}

void BoundingBoxItem::setCoordinates(const QRectF &coords) {
  setPos(coords.topLeft());
  setRect(QRectF(QPointF(0, 0), coords.size()));
}
