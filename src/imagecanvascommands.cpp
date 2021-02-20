#include "imagecanvascommands.h"

AddBBoxCommand::AddBBoxCommand(ImageCanvas *image_canvas,
                               BoundingBoxItem *new_bbox_item,
                               QUndoCommand *parent)
    : QUndoCommand(parent), m_imageCanvas(image_canvas),
      m_bboxItem(new_bbox_item) {
  m_label = m_bboxItem->label();
  m_bbox = m_bboxItem->boundingBoxCoordinates();
}

void AddBBoxCommand::undo() { m_imageCanvas->removeItem(m_bboxItem); }

void AddBBoxCommand::redo() {
  m_bboxItem->setPos(m_bbox.topLeft());
  m_bboxItem->setRect(0, 0, m_bbox.width(), m_bbox.height());
  m_imageCanvas->addItem(m_bboxItem);
}
