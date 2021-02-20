#ifndef IMAGECANVASCOMMANDS_H
#define IMAGECANVASCOMMANDS_H
#include "bboxitem.h"
#include "imagecanvas.h"
#include <QUndoCommand>

class AddBBoxCommand : public QUndoCommand {
public:
  AddBBoxCommand(ImageCanvas *image_canvas, BoundingBoxItem *new_bbox_item,
                 QUndoCommand *parent = nullptr);
  void undo() override;
  void redo() override;

private:
  ImageCanvas *m_imageCanvas;
  BoundingBoxItem *m_bboxItem;
  QString m_label;
  QRectF m_bbox;
};

#endif // IMAGECANVASCOMMANDS_H
