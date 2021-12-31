#include "undo_cmds.h"

AddBBoxCommand::AddBBoxCommand(const QRectF &rect, const QString &label, bool ready, ImageCanvas *canvas, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_item = new BoundingBoxItem(rect,label,nullptr,ready);
    m_canvas = canvas;
    m_bbox = rect;
}

void AddBBoxCommand::undo(){
    m_canvas->removeItem(m_item);
    //m_canvas->update();
}

void AddBBoxCommand::redo(){
    m_item->setCoordinates(m_bbox);
    m_canvas->addItem(m_item);
    //m_canvas->update();
}

AddBBoxCommand::~AddBBoxCommand(){
    if (!m_item->scene()){
        delete m_item;
    }
}

MoveBBoxCommand::MoveBBoxCommand(const QRectF &oldRect, const QRectF &newRect, BoundingBoxItem *item, QUndoCommand *parent)
    : QUndoCommand(parent), m_oldRect(oldRect), m_newRect(newRect), m_item(item){}

void MoveBBoxCommand::undo(){
    m_item->setCoordinates(m_oldRect);
}

void MoveBBoxCommand::redo(){
    m_item->setCoordinates(m_newRect);
}


SizeChangeBBoxCommand::SizeChangeBBoxCommand(const QRectF &oldRect, const QRectF &newRect, BoundingBoxItem *item, QUndoCommand *parent)
    : QUndoCommand(parent), m_oldRect(oldRect), m_newRect(newRect), m_item(item){}

void SizeChangeBBoxCommand::undo(){
    m_item->setCoordinates(m_oldRect);
}

void SizeChangeBBoxCommand::redo(){
    m_item->setCoordinates(m_newRect);
}
