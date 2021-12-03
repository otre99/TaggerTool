#include "graphicsview.h"

#include <QDebug>
#include <QScrollBar>
#include <QWheelEvent>

#include "imagecanvas.h"
GraphicsView::GraphicsView(QWidget *parent) : QGraphicsView(parent) {
  setTransformationAnchor(QGraphicsView::NoAnchor);
  setRenderHint(QPainter::Antialiasing);
}

void GraphicsView::wheelEvent(QWheelEvent *event) {
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  const QPoint numDegrees = event->angleDelta();
  if (numDegrees.y() > 0) {
    scale(1.1, 1.1);
  } else {
    scale(1.0 / 1.1, 1.0 / 1.1);
  }
  event->accept();
  setTransformationAnchor(QGraphicsView::NoAnchor);
  Helper::setScale(1.0 / transform().m11());
  dynamic_cast<ImageCanvas *>(scene())->helperParametersChanged();
}

void GraphicsView::mousePressEvent(QMouseEvent *event) {
  if ((event->modifiers() == Qt::ControlModifier) &&
      (event->buttons() == Qt::LeftButton)) {
    m_originX = event->pos().x();
    m_originY = event->pos().y();
  } else {
    QGraphicsView::mousePressEvent(event);
  }
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event) {
  if (event->modifiers() == Qt::ControlModifier &&
      event->buttons() == Qt::LeftButton) {
    QPointF oldP = mapToScene(m_originX, m_originY);
    QPointF newP = mapToScene(event->pos());
    QPointF translation = newP - oldP;

    translate(translation.x(), translation.y());

    m_originX = event->pos().x();
    m_originY = event->pos().y();
  } else {
    QGraphicsView::mouseMoveEvent(event);
  }
}
