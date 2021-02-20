#ifndef BBOX_H
#define BBOX_H
#include <QGraphicsRectItem>

class ImageCanvas;
class BoundingBoxItem : public QGraphicsRectItem {
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
    kLeftCenter
  } m_currentCorner;

public:
  BoundingBoxItem(const QRectF &rectf, const QString &label = QString(),
                  QGraphicsItem *parent = nullptr, bool ready = false);
  QRectF boundingBoxCoordinates();
  QString label() { return m_info; }
  void setLocked(bool what);

  void setLabel(const QString &lb);
  void setCoordinates(const QRectF &coords);

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

private:
  CORNER positionInsideBBox(const QPointF &pos);
  QRectF buildRectFromTwoPoints(const QPointF &p1, const QPointF &p2, bool &sw,
                                bool &sh);

  QString m_info;
  QPointF m_lastPt;
  bool m_showCorners;
  bool m_moveEnable;
  double m_marginW;
  double m_marginH;
};

#endif // BBOX_H
