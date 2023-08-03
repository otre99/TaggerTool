#ifndef CIRCLE_ITEM_H
#define CIRCLE_ITEM_H
#include <QGraphicsEllipseItem>

#include "custom_item.h"
#include "utils.h"

class ImageCanvas;
class CircleItem : public QGraphicsEllipseItem, public CustomItem {
  friend class ImageCanvas;
  enum CORNER { kCenter = -1, kBorder, kInvalid } m_currentCorner{kInvalid};

public:
  CircleItem(const QPointF &center, double radius,
             const QString &label = QString(), QGraphicsItem *parent = nullptr,
             bool ready = false);
  // CustomItem
  void helperParametersChanged() override;
  void setLocked(bool what) override { __setLocked(this, what); }
  void setLabel(const QString &lb) override { __setLabel(this, lb); }
  void setShowLabel(bool show) override {
    m_showLabel = show;
    update();
  }

  // QGraphicsItem
  int type() const override { return Helper::kCircle; }
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  //    void keyPressEvent(QKeyEvent *event) override;
  QRectF boundingRect() const override;

  // get/set
  QPointF center() const;
  qreal radius() const;

private:
  CORNER positionInside(const QPointF &pos);

  QRectF m_oldRect;
  QPointF m_oldPos;
  QPointF m_pivotPos;
  QPointF m_lastPt;
};

#endif // CIRCLE_ITEM_H
