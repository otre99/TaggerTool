#ifndef IMAGECANVASCOMMANDS_H
#define IMAGECANVASCOMMANDS_H
#include <QUndoCommand>
#include "imagecanvas.h"
#include "bboxitem.h"


class AddBBoxCommand : public QUndoCommand
{
public:

    AddBBoxCommand(ImageCanvas *image_canvas, BoundingBoxItem *new_bbox_item, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    ImageCanvas *image_canvas_;
    BoundingBoxItem *bbox_item_;
    QString label_;
    QRectF bbox_;
};

#endif // IMAGECANVASCOMMANDS_H
