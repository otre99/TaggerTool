#ifndef POINT_ITEM_H
#define POINT_ITEM_H
#include <QGraphicsEllipseItem>

#include "custom_item.h"
#include "utils.h"

class PointItem : public QGraphicsEllipseItem, public CustomItem {
 public:
  PointItem(const QPointF &center, const QString &label = QString(),
            QGraphicsItem *parent = nullptr, bool ready = false);
  int type() const override { return Helper::kPoint; }
  void setLocked(bool what) override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;
  void setLabel(const QString &lb) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
                      const QVariant &value) override;
  QRectF boundingRect() const override;

  QPointF center() const;
  void setShowLabel(bool show) override {
    m_showLabel = show;
    update();
  }

 private:
};

#endif  // POINT_ITEM_H
