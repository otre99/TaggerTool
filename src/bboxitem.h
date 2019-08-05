#ifndef BBOX_H
#define BBOX_H
#include <QGraphicsRectItem>

class ImageCanvas;
class BoundingBoxItem : public QGraphicsRectItem
{
    friend class ImageCanvas;
    enum CORNER {kCenter=-1, kTopLeft=0, kTopRight=1, kBottomRight=2, kBottomLeft=3, kTopCenter, kRightCenter, kBottomCenter, kLeftCenter} current_corner_;

public:
    BoundingBoxItem(const QRectF &rectf, const QString &label=QString(), QGraphicsItem *parent=nullptr, bool ready=false);
    QRectF BoundingBoxCoordinates();
    QString GetLabel() {return info_;}
    void SetLocked(bool what);

    void SetLabel(const QString &lb);
    void SetCoordinates(const QRectF &coords);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    QRectF boundingRect() const override;
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

private:
    CORNER PositionInsideBBox(const QPointF &pos);
    QRectF BuildRectFromTwoPoints(const QPointF &p1, const QPointF &p2, bool &sw, bool &sh);

    QString info_;
    QPointF last_point_;
    bool show_corners_;
    bool move_enable_;
    double margin_w_;
    double margin_h_;

};

#endif // BBOX_H
