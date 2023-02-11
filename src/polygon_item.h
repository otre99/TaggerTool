#ifndef POLYGON_ITEM_H
#define POLYGON_ITEM_H

#include <QGraphicsPolygonItem>

#include "custom_item.h"
#include "utils.h"

class ImageCanvas;
class QUndoCommand;

class PolygonItem : public QGraphicsPolygonItem, public CustomItem {
  friend class ImageCanvas;
  enum CORNER { kCenter = -1, kNode, kInvalid } m_currentCorner{kInvalid};
  int m_currentNodeIndx_;

public:
  PolygonItem(const QPolygonF &poly, const QString &label = QString(),
              QGraphicsItem *parent = nullptr, bool ready = false,
              bool closed_poly = true);
  // CustomItem
  void helperParametersChanged() override;
  void setLocked(bool what) override { __setLocked(this, what); }
  void setLabel(const QString &lb) override { __setLabel(this, lb); }
  void setShowLabel(bool show) override {
    m_showLabel = show;
    update();
  }

  // QGraphicsItem
  int type() const override {
    return m_closed_poly ? Helper::kPolygon : Helper::kLineStrip;
  }
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
  QPolygonF getPolygonCoords() const;

private:
  CORNER positionInside(const QPointF &pos);
  QPolygonF m_oldPolygon;
  QPointF m_oldPos;
  bool m_closed_poly;

  QUndoCommand *makeChangeCommand(const QPolygonF &oldPoly,
                                  const QPolygonF &newPoly);
};

#endif // POLYGON_ITEM_H
