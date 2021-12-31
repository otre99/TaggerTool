#ifndef BBOX_ITEM_H
#define BBOX_ITEM_H
#include <QGraphicsRectItem>

#include "custom_item.h"
#include "utils.h"

class ImageCanvas;
class BoundingBoxItem : public QGraphicsRectItem, public CustomItem {
  friend class ImageCanvas;
  enum CORNER {
    kCenter = -1,
    kTopLeft,
    kTopRight,
    kBottomRight,
    kBottomLeft,
    kTopCenter,
    kRightCenter,
    kBottomCenter,
    kLeftCenter,
    kInvalid
  } m_currentCorner;

 public:
  BoundingBoxItem(const QRectF &rectf, const QString &label = QString(),
                  QGraphicsItem *parent = nullptr, bool ready = false);

  // CustomItem
  void helperParametersChanged() override;
  void setLocked(bool what) override { __setLocked(this, what); }
  void setLabel(const QString &lb) override { __setLabel(this, lb); }
  void setShowLabel(bool show) override {
    m_showLabel = show;
    update();
  }

  // QGraphicsItem
  int type() const override { return Helper::kBBox; }
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  QPainterPath shape() const override;
  QRectF boundingRect() const override;
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
                      const QVariant &value) override;

  // get/set
  QRectF boundingBoxCoordinates();
  void setCoordinates(const QRectF &coords);

 private:
  QRectF buildRectFromTwoPoints(const QPointF &p1, const QPointF &p2, bool &sw,
                                bool &sh);
  CORNER positionInside(const QPointF &pos);

  QPointF m_lastPt;
  QRectF m_oldCoords;
};

#endif  // BBOX_ITEM_H
