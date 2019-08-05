#ifndef IMAGECANVAS_H
#define IMAGECANVAS_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include "bboxitem.h"
#include <QGraphicsTextItem>
#include <QPropertyAnimation>
#include <QDebug>
#include <QUndoStack>

class NiceText : public QGraphicsTextItem
{
    QPropertyAnimation anim;
public:
    NiceText() {
        anim.setTargetObject(this);
        anim.setPropertyName("opacity");
        anim.setKeyValueAt(0.0,0.0);
        anim.setKeyValueAt(0.5,1.0);
        anim.setKeyValueAt(1.0,0.0);
        anim.setDuration(1200);
    }

    void display() {
        setOpacity(0.0);
        setVisible(true);
        anim.start();
    }
};


class ImageCanvas : public QGraphicsScene
{
    Q_OBJECT
private:
    QPixmap current_image_;
    QString image_id_;
    QSizeF default_bbox_size_;
    QString bbox_label_;
    bool waiting_for_new_bbox_;
    bool draw_box_started_;
    QPointF beg_pt_;
    QPointF end_pt_;
    bool need_save_changes_;
    NiceText display_text_;
    //TODO (otre99): to allow Ctr+Z
    //QUndoStack undoStack_;

public:
    void ShowNiceText(const QString &txt);
    ImageCanvas(QObject *parent=nullptr);
    void Reset(const QImage &img, const QString &img_id);
    void AddBoundingBoxes(const QList<QRectF> &bboxes, const QStringList &labels=QStringList());
    QMap<QString, QList<QRectF>> GetBoundingBoxes();
    void RemoveSelectedBoundingBoxes();
    void Clear();
    QSize GetImageSize();
    QString GetImageId();
    void HideBoundingBoxes();
    void ShowBoundingBoxes();
    void PrepareForNewBBox(QString label=QString(), QSizeF bboxSize=QSizeF() , bool what=true);


    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

signals:
    void BBoxItemToEditor(QGraphicsItem *iten, int reason);
};

#endif // IMAGECANVAS_H
