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

  void setOccluded(bool occluded) { m_occluded = occluded; }
  void setTruncated(bool occluded) { m_truncated = occluded; }
  void setCrowded(bool crowded) { m_crowded = crowded; }

  bool getOccluded() const { return m_occluded; }
  bool getTruncated() const { return m_truncated; }
  bool getCrowded() const { return m_crowded; }

  virtual void showEditDialog(QGraphicsItem *item,
                              const QPoint screenPos) override;

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

  // get/set
  QRectF boundingBoxCoordinates();

private:
  QRectF buildRectFromTwoPoints(const QPointF &p1, const QPointF &p2, bool &sw,
                                bool &sh);
  CORNER positionInside(const QPointF &pos);

  QPointF m_oldPos;
  QPointF m_lastPt;
  QRectF m_oldCoords;
  bool m_occluded{false}, m_truncated{false}, m_crowded{false};
};

#endif // BBOX_ITEM_H
