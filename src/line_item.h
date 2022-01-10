#ifndef LINE_ITEM_H
#define LINE_ITEM_H

#include <QGraphicsLineItem>

#include "custom_item.h"
#include "utils.h"

class ImageCanvas;

class LineItem : public QGraphicsLineItem, public CustomItem {
  friend class ImageCanvas;
  enum CORNER { kCenter = -1, kP1 = 0, kP2 = 1, kInvalid } m_currentCorner;

 public:
  LineItem(const QPointF &p1, const QPointF &p2,
           const QString &label = QString(), QGraphicsItem *parent = nullptr,
           bool ready = false);
  // CustomItem
  void helperParametersChanged() override;
  void setLocked(bool lk) override { __setLocked(this, lk); };
  void setLabel(const QString &lb) override;
  void setShowLabel(bool show) override {
    m_showLabel = show;
    update();
  }

  // QGraphicsItem
  int type() const override { return Helper::kLine; }
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  QRectF boundingRect() const override;

  // get/set
  QPointF p1() const { return mapToScene(line().p1()); }
  QPointF p2() const { return mapToScene(line().p2()); }

 private:
  CORNER positionInside(const QPointF &pos);
  QPointF m_oldPos;
  QLineF m_oldLine;
};

#endif  // LINE_ITEM_H
