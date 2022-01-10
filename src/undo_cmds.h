#ifndef UNDO_CMDS_H
#define UNDO_CMDS_H
#include <QGraphicsItem>
#include <QLineF>
#include <QPolygonF>
#include <QRectF>
#include <QUndoCommand>

class BoundingBoxItem;
class PolygonItem;
class PointItem;
class LineItem;

/////////////////////////////////////////////////////////////////
//////////////// BoundingBoxItem ///////////////////////////////
/////////////////////////////////////////////////////////////////
// AddBBoxCommand
class AddBBoxCommand : public QUndoCommand {
 public:
  AddBBoxCommand(const QRectF &rect, const QString &label, bool ready,
                 QUndoCommand *parent = 0);
  ~AddBBoxCommand();
  void undo() override;
  void redo() override;

 private:
  BoundingBoxItem *m_item;
};

class SizeChangeBBoxCommand : public QUndoCommand {
 public:
  SizeChangeBBoxCommand(const QRectF &oldRect, const QRectF &newRect,
                        BoundingBoxItem *item, QUndoCommand *parent = 0);
  void undo() override;
  void redo() override;

 private:
  QRectF m_oldRect, m_newRect;
  BoundingBoxItem *m_item;
};

/////////////////////////////////////////////////////////////////
///////////////////// PolygonItem ///////////////////////////////
/////////////////////////////////////////////////////////////////
// AddPolygonCommand
class AddPolygonCommand : public QUndoCommand {
 public:
  AddPolygonCommand(const QPolygonF &poly, const QString &label, bool ready,
                    QUndoCommand *parent = 0);
  ~AddPolygonCommand();

  void undo() override;
  void redo() override;

 private:
  PolygonItem *m_item;
};

// ChangePolygonCommand
class ChangePolygonShapeCommand : public QUndoCommand {
 public:
  ChangePolygonShapeCommand(const QPolygonF &oldPoly, const QPolygonF &newPoly,
                            PolygonItem *item, QUndoCommand *parent = 0);
  void undo() override;
  void redo() override;

 private:
  QPolygonF m_oldPoly, m_newPoly;
  ;
  PolygonItem *m_item;
  QString m_label;
};

/////////////////////////////////////////////////////////////////
//////////////////// PointItem //////////////////////////////////
/////////////////////////////////////////////////////////////////
// AddPolygonCommand
class AddPointCommand : public QUndoCommand {
 public:
  AddPointCommand(const QPointF &pos, const QString &label, bool ready,
                  QUndoCommand *parent = 0);
  ~AddPointCommand();
  void undo() override;
  void redo() override;

 private:
  PointItem *m_item;
};

/////////////////////////////////////////////////////////////////
//////////////////// LineItem ///////////////////////////////////
/////////////////////////////////////////////////////////////////
// AddLineCommand
class AddLineCommand : public QUndoCommand {
 public:
  AddLineCommand(const QPointF &p1, const QPointF &p2, const QString &label,
                 bool ready, QUndoCommand *parent = 0);
  ~AddLineCommand();
  void undo() override;
  void redo() override;

 private:
  LineItem *m_item;
};

// ChangeLineSizeCommand
class ChangeLineSizeCommand : public QUndoCommand {
 public:
  ChangeLineSizeCommand(const QLineF &oldLine, const QLineF &newLine,
                        LineItem *item, QUndoCommand *parent = 0);
  void undo() override;
  void redo() override;

 private:
  LineItem *m_item;
  QLineF m_oldLine, m_newLine;
};

/////////////////////////////////////////////////////////////////
////////////////////////// commond //////////////////////////////
/////////////////////////////////////////////////////////////////
// MoveItemCommand
class MoveItemCommand : public QUndoCommand {
 public:
  MoveItemCommand(const QPointF &oldPos, const QPointF &newPos,
                  QGraphicsItem *item, QUndoCommand *parent = 0);
  void undo() override;
  void redo() override;

 private:
  QPointF m_oldPos, m_newPost;
  QGraphicsItem *m_item;
};

// RemoveItemCommand
class RemoveItemCommand : public QUndoCommand {
 public:
  RemoveItemCommand(QGraphicsItem *item, QUndoCommand *parent = 0);
  void undo() override;
  void redo() override;

 private:
  QGraphicsItem *m_item;
};

// RemoveItemCommand
class ChangeLabelCommand : public QUndoCommand {
 public:
  ChangeLabelCommand(const QString &oldLabel, const QString &newLabel,
                     QGraphicsItem *item, QUndoCommand *parent = 0);
  void undo() override;
  void redo() override;

 private:
  QString m_oldLb, m_newLb;
  QGraphicsItem *m_item;
  void setLabel(const QString &lb);
};

#endif  // UNDO_CMDS_H
