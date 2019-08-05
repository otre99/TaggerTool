#include "imagecanvas.h"
#include <QPainter>
#include <QDebug>
#include "bboxitem.h"
#include <QGraphicsView>
#include "utils.h"
#include "imagecanvascommands.h"

ImageCanvas::ImageCanvas(QObject *parent) : QGraphicsScene (parent),
    default_bbox_size_(20,20),
    waiting_for_new_bbox_(false),
    draw_box_started_(false)
{
    addItem(&display_text_);
}

void ImageCanvas::HideBoundingBoxes()
{
    for (auto item : items()){
        if ( !item->isSelected() ){
            item->setVisible(false);
        }
    }
}

void ImageCanvas::ShowNiceText(const QString &txt)
{
    QGraphicsView *view = views().first();
    QPointF pos = view->mapToScene( QPoint(10, 10) );
    display_text_.setPos( pos );
    display_text_.setPlainText(txt);
    display_text_.display();
}

void ImageCanvas::PrepareForNewBBox(QString label, QSizeF bboxSize, bool what)
{
    waiting_for_new_bbox_ = what;
    if ( label != QString() ) bbox_label_ = label;
    if (bboxSize != QSizeF() ) default_bbox_size_ = bboxSize;
    views().first()->viewport()->setCursor(Qt::CrossCursor);
}


void ImageCanvas::ShowBoundingBoxes()
{
    for (auto item : items()){
        item->setVisible(true);
    }
}

void ImageCanvas::drawBackground(QPainter *painter, const QRectF &rect)
{
    if ( !current_image_.isNull() )
        painter->drawPixmap(0,0,current_image_);
}

void ImageCanvas::Reset(const QImage &img, const QString &img_id)
{
    current_image_ = QPixmap::fromImage(img);
    setSceneRect(0,0,current_image_.width(), current_image_.height());
    Clear();
    image_id_ = img_id;
    update();
}

void ImageCanvas::AddBoundingBoxes(const QList<QRectF> &bboxes, const QStringList &labels)
{
    if ( labels.isEmpty() ){
        for (const auto &rect : bboxes){
            QRectF frect = sceneRect()&rect;
            if ( frect.isNull() || frect.isEmpty() || !frect.isValid() )
                continue;
            addItem( new BoundingBoxItem(frect) );
        }
    } else if ( labels.count() != bboxes.count() ) {
        for (const auto &rect : bboxes){
            QRectF frect = sceneRect()&rect;
            if ( frect.isNull() || frect.isEmpty() || !frect.isValid() )
                continue;
            addItem( new BoundingBoxItem(frect, labels[0]) );
        }
    } else {
        for (int i=0; i<bboxes.count(); ++i){
            QRectF frect = sceneRect()&bboxes[i];
            if ( frect.isNull() || frect.isEmpty() || !frect.isValid() )
                continue;
            addItem( new BoundingBoxItem(frect, labels[i]) );
        }
    }
}

void ImageCanvas::RemoveSelectedBoundingBoxes()
{
    for (auto *item : selectedItems()){
        removeItem(item);
        delete  item;
    }
}

void ImageCanvas::Clear()
{
    for (auto *item :items()){
        auto *to_del = dynamic_cast<BoundingBoxItem*>(item);
        if (to_del){
            removeItem(item);
            delete  item;
        }
    }
}


QMap<QString, QList<QRectF> > ImageCanvas::GetBoundingBoxes()
{
    QMap<QString, QList<QRectF>> result;
    for (const auto &item : items()){
        auto *bbox = dynamic_cast<BoundingBoxItem*>(item);
        if (bbox){
            result[bbox->info_].append( bbox->BoundingBoxCoordinates() );
        }
    }
    return  result;
}

QSize ImageCanvas::GetImageSize()
{
    return current_image_.size();
}
QString ImageCanvas::GetImageId()
{
    return image_id_;
}


void ImageCanvas::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (waiting_for_new_bbox_){
        end_pt_ = beg_pt_ = mouseEvent->scenePos();
        draw_box_started_ = true;
        waiting_for_new_bbox_ = false;
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void ImageCanvas::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (draw_box_started_){
        end_pt_ = mouseEvent->scenePos();
        update();
    }
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}
void ImageCanvas::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (draw_box_started_){
        draw_box_started_  = false;
        views().first()->viewport()->setCursor(Qt::ArrowCursor);
        if (beg_pt_ == end_pt_) return;

        auto bbox  = Helper::BuildRectFromTwoPoints(beg_pt_, end_pt_);
        auto *item = new BoundingBoxItem(bbox, bbox_label_,nullptr,true);
        addItem(item);
        //undoStack_.push(new AddBBoxCommand(this, item) );


        update();
    }
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void ImageCanvas::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawForeground(painter, rect);
    if (draw_box_started_)
        painter->drawRect( Helper::BuildRectFromTwoPoints(beg_pt_, end_pt_) );
}


