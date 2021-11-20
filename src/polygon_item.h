#ifndef POLYGON_ITEM_H
#define POLYGON_ITEM_H

#include <QGraphicsPolygonItem>

#include "custom_item.h"
#include "utils.h"

class ImageCanvas;

class PolygonItem : public QGraphicsPolygonItem, public CustomItem {
  friend class ImageCanvas;
  enum CORNER { kCenter = -1, kNode, kInvalid} m_currentCorner;
  int m_currentNodeIndx_;

public:
  PolygonItem(const QPolygonF &poly, const QString &label = QString(), QGraphicsItem *parent = nullptr, bool ready = false);
  int type() const override { return Helper::kPolygon; }
  void setLocked(bool what) override;
  void setLabel(const QString &lb) override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

  QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
                      const QVariant &value) override;
  QRectF boundingRect() const override;
  QPainterPath shape() const override;
  void setShowLabel(bool show) override {
    m_showLabel = show;
    update();
  }

 private:
    CORNER positionInsideBBox(const QPointF &pos);
};

#endif  // POLYGON_ITEM_H
