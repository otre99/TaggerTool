#include "bboxitem.h"
#include <QKeyEvent>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QGraphicsScene>
#include <QCursor>
#include "imagecanvas.h"
#include "utils.h"

namespace {
const QColor kUnlockedBBoxColor(0,0,128,64);
const QColor kLockedBBoxColor(128,128,128,64);
const QColor kMarginBBoxColor(128,0,0,64);
const QColor kLabelColor(200,200,200,64);
const int kLabelRectSize=12;
}

BoundingBoxItem::BoundingBoxItem(const QRectF &rectf, const QString &label,  QGraphicsItem *parent, bool ready) :
    QGraphicsRectItem (parent),
    show_corners_(ready),
    move_enable_(ready)
{    
    setFlags(QGraphicsItem::ItemIsFocusable|QGraphicsItem::ItemSendsGeometryChanges);
    setPos(rectf.topLeft());
    setRect(QRectF(0,0,rectf.width(), rectf.height()));

    SetLocked(!ready);
    if (ready){
        setSelected(ready);
    }
    SetLabel(label);
    setAcceptHoverEvents(true);
}

QRectF BoundingBoxItem::BoundingBoxCoordinates()
{
    const QSizeF  sf = rect().size();
    const QPointF tl = pos();
    return {tl.x(), tl.y(), sf.width(), sf.height()};
}

QRectF BoundingBoxItem::boundingRect() const
{
    const QRectF r = QGraphicsRectItem::boundingRect();
    return r;
}

void BoundingBoxItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);
    const QRectF brect = boundingRect();

    if ( !move_enable_ || !isSelected() ) {
        QRectF lb_rect;
        lb_rect.setTopLeft( brect.topLeft() );
        lb_rect.setWidth( brect.width());
        lb_rect.setHeight( kLabelRectSize );
        painter->fillRect(lb_rect,kLabelColor);

        QFont font = painter->font();
        font.setPixelSize(11);
        painter->setFont(font);
        painter->drawText(lb_rect, Qt::AlignCenter, info_);

    }


    if ( move_enable_ ){
        const double ww = brect.width();
        const double hh = brect.height();
        margin_w_ = qMax(10.0, qMin(50.0, ww*0.1));
        margin_h_ = qMax(10.0, qMin(50.0, hh*0.1));
        margin_w_ = margin_h_ = qMin(margin_w_,margin_h_);

        if ( qMin(ww,hh) > 10 ){
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(kMarginBBoxColor) );

            switch (current_corner_) {
            case kTopLeft:
                painter->drawRect(QRectF(brect.x()+0.5, brect.y()+0.5, margin_w_, margin_h_));
                break;
            case kTopCenter:
                painter->drawRect(QRectF(QPointF(brect.x()+margin_w_-0.5, brect.y()+0.5),
                                         QPointF(brect.x()+brect.width()-margin_w_-0.5, brect.y()+0.5+margin_h_)));
                break;
            case kTopRight:
                painter->drawRect(QRectF(brect.x()+brect.width()-margin_w_-0.5, brect.y()+0.5, margin_w_, margin_h_));
                break;
            case kRightCenter:
                painter->drawRect(QRectF(QPointF(brect.x()+brect.width()-margin_w_-0.5, brect.y()+0.5+margin_h_),
                                         QPointF(brect.x()+brect.width()-0.5, brect.y()+brect.height()-0.5-margin_h_)));
                break;
            case kBottomRight:
                painter->drawRect(QRectF(brect.x()+brect.width()-margin_w_-0.5, brect.y()+brect.height()-0.5-margin_h_, margin_w_, margin_h_));
                break;
            case kBottomCenter:
                painter->drawRect(QRectF(QPointF(brect.x()+margin_w_+0.5, brect.y()+brect.height()-0.5-margin_h_),
                                         QPointF(brect.x()+brect.width()-0.5-margin_w_, brect.y()+brect.height()-0.5)));
                break;
            case kBottomLeft:
                painter->drawRect(QRectF(brect.x()+0.5, brect.y()+brect.height()-0.5-margin_h_, margin_w_, margin_h_));
                break;
            case kLeftCenter:
                painter->drawRect(QRectF(QPointF(brect.x()+0.5, brect.y()+0.5+margin_h_),
                                         QPointF(brect.x()+margin_w_+0.5, brect.y()+brect.height()-0.5-margin_h_)));
                break;
            }
        }
    }
}

void BoundingBoxItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (current_corner_ == kCenter || !move_enable_)
        QGraphicsRectItem::mouseMoveEvent(event);
    else {
        QPointF cpos = mapToScene(event->pos());
        QPointF dl = cpos - last_point_;

        QRectF srect = scene()->sceneRect();
        QRectF newrect(this->pos(), this->rect().size());

        CORNER new_corner = current_corner_;
        bool sw;
        bool sh;

        switch (current_corner_) {
        case kTopLeft:
            newrect = BuildRectFromTwoPoints(newrect.topLeft()+dl, newrect.bottomRight(), sw, sh);
            if ( sw && sh ) new_corner = kBottomRight;
            else if ( sw )  new_corner = kTopRight;
            else if ( sh )  new_corner = kBottomLeft;
            break;
        case kTopRight:
            newrect = BuildRectFromTwoPoints(newrect.topRight()+dl, newrect.bottomLeft(), sw, sh);
            if ( !sw && sh ) new_corner = kBottomLeft;
            else if ( !sw )  new_corner = kTopLeft;
            else if ( sh )  new_corner = kBottomRight;
            break;
        case kBottomRight:
            newrect = BuildRectFromTwoPoints(newrect.bottomRight()+dl, newrect.topLeft(), sw, sh);
            if ( !sw && !sh ) new_corner = kTopLeft;
            else if ( !sw )  new_corner = kBottomLeft;
            else if ( !sh )  new_corner = kTopRight;
            break;
        case kBottomLeft:
            newrect = BuildRectFromTwoPoints(newrect.bottomLeft()+dl, newrect.topRight(), sw, sh);
            if ( sw && !sh ) new_corner = kTopRight;
            else if ( sw )  new_corner = kBottomRight;
            else if ( !sh )  new_corner = kTopLeft;
            break;
        case kTopCenter:
            dl.setX(0);
            newrect = BuildRectFromTwoPoints(newrect.topLeft()+dl, newrect.bottomRight(), sw, sh);
            if ( sh ) new_corner = kBottomCenter;
            break;
        case kBottomCenter:
            dl.setX(0);
            newrect = BuildRectFromTwoPoints(newrect.bottomRight()+dl, newrect.topLeft(), sw, sh);
            if ( !sh ) new_corner = kTopCenter;
            break;
        case kRightCenter:
            dl.setY(0);
            newrect = BuildRectFromTwoPoints(newrect.topRight()+dl, newrect.bottomLeft(), sw, sh);
            if ( !sw ) new_corner = kLeftCenter;
            break;
        case kLeftCenter:
            dl.setY(0);
            newrect = BuildRectFromTwoPoints(newrect.topLeft()+dl, newrect.bottomRight(), sw, sh);
            if ( sw ) new_corner = kRightCenter;
            break;
        }

        current_corner_ = new_corner;
        newrect&=srect;
        if ( newrect.isValid()){
            prepareGeometryChange();
            setPos(newrect.topLeft());
            setRect(0,0,newrect.width(), newrect.height() ) ;
            last_point_ =  cpos;
        }
    }
    if ( isSelected() ) emit dynamic_cast<ImageCanvas*>( scene() )->BBoxItemToEditor(this, 0);
}

QRectF BoundingBoxItem::BuildRectFromTwoPoints(const QPointF &p1, const QPointF &p2, bool &sw, bool &sh)
{
    sw = p2.x() < p1.x();
    sh = p2.y() < p1.y();
    double rw = qAbs(p2.x()-p1.x());
    double rh = qAbs(p2.y()-p1.y());
    QPointF topleft( qMin(p1.x(), p2.x()), qMin(p1.y(),p2.y()) );
    return {topleft, QSizeF(rw,rh)};
}

void BoundingBoxItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier) )    {
        QPointF pt = mapToScene(event->pos());
        const auto l = scene()->items(pt);
        if (l.count() > 1) {
            for (int i=1; i<l.count(); ++i) this->stackBefore( l[i] );
            auto* bbox = dynamic_cast<BoundingBoxItem*>( l[1] );
            if (bbox){
                bbox->SetLocked(false);
                this->SetLocked(true);
            }
        }
    } else if (event->modifiers() ==  Qt::ShiftModifier){
        SetLocked( move_enable_ );
    } else {
        current_corner_ = PositionInsideBBox(event->pos());
        if ( current_corner_ == kCenter || !move_enable_){
            QGraphicsRectItem::mousePressEvent(event);
        } else {
            update();
            last_point_ = mapToScene(event->pos());
        }
    }
}

void BoundingBoxItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsRectItem::mouseDoubleClickEvent(event);

}

void BoundingBoxItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::ArrowCursor);
    QGraphicsRectItem::mouseReleaseEvent(event);
    if ( isSelected() ) emit dynamic_cast<ImageCanvas*>( scene() )->BBoxItemToEditor(this, 0);
}

BoundingBoxItem::CORNER BoundingBoxItem::PositionInsideBBox(const QPointF &pos)
{
    const double x = pos.x();
    const double y = pos.y();
    CORNER result=kCenter;
    const QRectF brect = boundingRect();
    if ( (brect.x() <= x) && (x <= brect.x() + margin_w_) &&  (brect.y() <= y) && (y <= brect.y() + margin_h_) ){
        result = kTopLeft;
        if (move_enable_) setCursor(Qt::SizeFDiagCursor);
    } else if ( (brect.x()+brect.width()-margin_w_ <= x) && (x <= brect.x()+brect.width()) &&  (brect.y() <= y) && (y <= brect.y() + margin_h_) ){
        result = kTopRight;
        if (move_enable_) setCursor(Qt::SizeBDiagCursor);
    } else if ( (brect.x()+brect.width()-margin_w_ <= x) && (x <= brect.x()+brect.width()) &&  (brect.y()+brect.height()-margin_h_ <= y) && (y <= brect.y()+brect.height()) ){
        result = kBottomRight;
        if (move_enable_) setCursor(Qt::SizeFDiagCursor);
    } else if ( (brect.x() <= x) && (x <= brect.x() + margin_w_) &&  (brect.y()+brect.height()-margin_h_ <= y) && (y <= brect.y()+brect.height()) ){
        result = kBottomLeft;
        if (move_enable_) setCursor(Qt::SizeBDiagCursor);
    } else if ( (brect.x() + margin_w_ <= x) && (x <= brect.x() + brect.width() - margin_w_) &&  (brect.y() <= y) && (y <= brect.y() + margin_h_) ){
        result = kTopCenter;
        if (move_enable_) setCursor(Qt::SizeVerCursor);
    } else if ( (brect.x()+brect.width()-margin_w_ <= x) && (x <= brect.x()+brect.width()) &&  (brect.y() + margin_h_ <= y) && (y <= brect.y()+brect.height() - margin_h_) ){
        result = kRightCenter;
        if (move_enable_) setCursor(Qt::SizeHorCursor);
    } else if ( (brect.x()+margin_w_ <= x) && (x <= brect.x()+brect.width()-margin_w_) && (brect.y()+brect.height()-margin_h_ <= y) && (y <= brect.y()+brect.height()) ){
        result = kBottomCenter;
        if (move_enable_) setCursor(Qt::SizeVerCursor);
    } else if ( (brect.x() <= x) && (x <= brect.x() + margin_w_) &&  (brect.y()+margin_h_ <= y) && (y <= brect.y()+brect.height()-margin_h_) ){
        result = kLeftCenter;
        if (move_enable_) setCursor(Qt::SizeHorCursor);
    } else {
        if (move_enable_) setCursor(Qt::SizeAllCursor);
    }
    return  result;
}

void BoundingBoxItem::keyPressEvent(QKeyEvent *event)
{
    if ( move_enable_ && ( event->key() == Qt::Key_Left || event->key()==Qt::Key_Right
                           || event->key()==Qt::Key_Up || event->key()==Qt::Key_Down)){
        QRectF newrect(this->pos(), this->rect().size());
        double dx1=0, dy1=0, dx2=0, dy2=0;
        double sx=1, sy=1;

        if ( event->modifiers()&Qt::ShiftModifier ){
            sx = std::max(newrect.width()*0.1, 1.0);
            sy = std::max(newrect.height()*0.1, 1.0);
        }

        if (event->modifiers()&Qt::ControlModifier){
            switch (event->key()) {
            case Qt::Key_Left: dx2=-sx; break;
            case Qt::Key_Right: dx2=+sx; break;
            case Qt::Key_Up: dy2=-sy; break;
            case Qt::Key_Down: dy2=sy; break;
            }
        } else {
            switch (event->key()) {
            case Qt::Key_Left: dx1=-sx; break;
            case Qt::Key_Right: dx1=+sx; break;
            case Qt::Key_Up: dy1=-sy; break;
            case Qt::Key_Down: dy1=sy; break;
            }
        }
        newrect.adjust(dx1, dy1, dx2, dy2);
        newrect&=scene()->sceneRect();
        if (newrect.isValid()){
            prepareGeometryChange();
            setPos(newrect.topLeft());
            setRect(0,0,newrect.width(), newrect.height() ) ;
        }
    } else if (event->key() == Qt::Key_Return) {
        this->SetLocked(true);
        //setCursor(Qt::ArrowCursor);
    } else
        QGraphicsItem::keyPressEvent(event);
    if ( isSelected() ) emit dynamic_cast<ImageCanvas*>( scene() )->BBoxItemToEditor(this, 0);
}

void BoundingBoxItem::SetLocked(bool what)
{
    move_enable_ = !what;
    setFlag(QGraphicsItem::ItemIsMovable, move_enable_);
    setFlag(QGraphicsItem::ItemIsSelectable, move_enable_);
    setFlag(QGraphicsItem::ItemIsFocusable, move_enable_);
    setBrush( move_enable_ ? kUnlockedBBoxColor : kLockedBBoxColor);
}

QVariant BoundingBoxItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    // TODO (rbt): I decided reimplement this function for some reason, but  I don't remember well!!!!!
    return value;
}

void BoundingBoxItem::SetLabel(const QString &lb)
{
    info_ = lb;
    setPen( Helper::ColorFromLabel(lb) );
    setToolTip(info_);
}

void BoundingBoxItem::SetCoordinates(const QRectF &coords)
{
    setPos(coords.topLeft());
    setRect(QRectF(QPointF(0,0),coords.size()));
}
