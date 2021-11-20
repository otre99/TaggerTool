#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QObject>
#include <QWidget>

class GraphicsView : public QGraphicsView {
  Q_OBJECT
 public:
  GraphicsView(QWidget* parent = nullptr);

 protected:
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

 private:
  int m_originX;
  int m_originY;
};

#endif  // GRAPHICSVIEW_H
