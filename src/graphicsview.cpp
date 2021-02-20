#include "graphicsview.h"
#include "imagecanvas.h"
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>
GraphicsView::GraphicsView(QWidget *parent) : QGraphicsView(parent) {}

void GraphicsView::wheelEvent(QWheelEvent *event) {
  const QPoint numDegrees = event->angleDelta();
  if (numDegrees.y() > 0) {
    scale(1.1, 1.1);
  } else {
    scale(1.0 / 1.1, 1.0 / 1.1);
  }
  event->accept();
}
