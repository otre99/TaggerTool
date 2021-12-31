#ifndef UNDO_CMDS_H
#define UNDO_CMDS_H
#include "imagecanvas.h"
#include "bbox_item.h"
#include <QRectF>

class AddBBoxCommand : public QUndoCommand
{
public:
    AddBBoxCommand(const QRectF &rect, const QString &label, bool ready, ImageCanvas *canvas, QUndoCommand *parent = 0);
    ~AddBBoxCommand();
    void undo() override;
    void redo() override;
private:
    QRectF m_bbox;
    ImageCanvas *m_canvas;
    BoundingBoxItem *m_item;
};

class MoveBBoxCommand : public QUndoCommand
{
public:
    MoveBBoxCommand(const QRectF &oldRect, const QRectF &newRect, BoundingBoxItem *item, QUndoCommand *parent = 0);
    void undo() override;
    void redo() override;

private:
    QRectF m_oldRect, m_newRect;
    BoundingBoxItem *m_item;
};

class SizeChangeBBoxCommand : public QUndoCommand
{
public:
    SizeChangeBBoxCommand(const QRectF &oldRect, const QRectF &newRect, BoundingBoxItem *item, QUndoCommand *parent = 0);
    void undo() override;
    void redo() override;

private:
    QRectF m_oldRect, m_newRect;
    BoundingBoxItem *m_item;
};

#endif // UNDO_CMDS_H
