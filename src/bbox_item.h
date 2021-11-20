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
    kTopLeft = 0,
    kTopRight = 1,
    kBottomRight = 2,
    kBottomLeft = 3,
    kTopCenter,
    kRightCenter,
    kBottomCenter,
    kLeftCenter,
    kInvalid
  } m_currentCorner;

 public:
  BoundingBoxItem(const QRectF &rectf, const QString &label = QString(),
                  QGraphicsItem *parent = nullptr, bool ready = false);
  QRectF boundingBoxCoordinates();
  void setCoordinates(const QRectF &coords);
  int type() const override { return Helper::kBBox; }

  void setLocked(bool what) override;
  void setLabel(const QString &lb) override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  QRectF boundingRect() const override;
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
                      const QVariant &value) override;

  void setShowLabel(bool show) override {
    m_showLabel = show;
    update();
  }

 private:
  CORNER positionInsideBBox(const QPointF &pos);
  QRectF buildRectFromTwoPoints(const QPointF &p1, const QPointF &p2, bool &sw,
                                bool &sh);

  QPointF m_lastPt;
  bool m_showCorners;
  double m_marginW;
  double m_marginH;
};

#endif  // BBOX_ITEM_H
