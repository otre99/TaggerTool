#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QObject>
#include <QWidget>
class GraphicsView : public QGraphicsView {
  Q_OBJECT
public:
  GraphicsView(QWidget *parent = nullptr);

protected:
  void wheelEvent(QWheelEvent *event);
};

#endif // GRAPHICSVIEW_H
