#include "imagecanvascommands.h"

AddBBoxCommand::AddBBoxCommand(ImageCanvas *image_canvas, BoundingBoxItem *new_bbox_item, QUndoCommand *parent) :
    QUndoCommand(parent), image_canvas_(image_canvas), bbox_item_(new_bbox_item)
{
    label_ = bbox_item_->GetLabel();
    bbox_  = bbox_item_->BoundingBoxCoordinates();
}


void AddBBoxCommand::undo()
{
    image_canvas_->removeItem(bbox_item_);
}

void AddBBoxCommand::redo()
{
    bbox_item_->setPos(bbox_.topLeft());
    bbox_item_->setRect(0,0,bbox_.width(), bbox_.height());
    image_canvas_->addItem(bbox_item_);
}


