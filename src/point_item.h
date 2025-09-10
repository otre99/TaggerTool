#ifndef POINT_ITEM_H
#define POINT_ITEM_H
#include <QGraphicsEllipseItem>

#include "custom_item.h"
#include "utils.h"
class ImageCanvas;

class PointItem : public QGraphicsEllipseItem, public CustomItem {
 public:
  PointItem(ImageCanvas *canvas, const QPointF &center,
            const QString &label = QString(), QGraphicsItem *parent = nullptr,
            bool ready = false);

  // CustomItem
  void helperParametersChanged() override;
  void setLocked(bool what) override { __setLocked(this, what); }
  void setLabel(const QString &lb) override { __setLabel(this, lb); }

  // QGraphicsItem
  int type() const override { return Helper::kPoint; }
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  QRectF boundingRect() const override;

  // get/set
  QPointF center() const;

 private:
  QPointF m_oldPos;
};

#endif  // POINT_ITEM_H
