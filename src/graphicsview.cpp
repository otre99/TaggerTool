#include "graphicsview.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QScrollBar>
#include "imagecanvas.h"
GraphicsView::GraphicsView(QWidget *parent) : QGraphicsView(parent)
{
}

void GraphicsView::wheelEvent(QWheelEvent *event) {
  const QPoint numDegrees = event->angleDelta();
  if (numDegrees.y() > 0) {
      scale(1.1,1.1);
  } else {
      scale(1.0/1.1,1.0/1.1);
  }
  event->accept();
}

