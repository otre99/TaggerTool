#include "undo_cmds.h"

#include "bbox_item.h"
#include "imagecanvas.h"
#include "line_item.h"
#include "point_item.h"
#include "polygon_item.h"
#include "circle_item.h"
#include "utils.h"

QString getBoolPropertyChangeText(const QString &pname, bool val1) {
  const auto arg1 = val1 ? "True" : "False";
  const auto arg2 = val1 ? "False" : "True";
  return QString("%1 [%2 --> %3]").arg(pname, arg1, arg2);
}

/////////////////////////////////////////////////////////////////
//////////////// BoundingBoxItem ////////////////////////////////
/////////////////////////////////////////////////////////////////
// AddBBoxCommand
AddBBoxCommand::AddBBoxCommand(const QRectF &rect, const QString &label,
                               bool occluded, bool truncated, bool crowded,
                               bool ready, QUndoCommand *parent)
    : QUndoCommand("AddBBox", parent) {
  m_item = new BoundingBoxItem(rect, label, nullptr, ready);
  m_item->setOccluded(occluded);
  m_item->setTruncated(truncated);
  m_item->setCrowded(crowded);
}

void AddBBoxCommand::undo() { Helper::imageCanvas()->removeItem(m_item); }

void AddBBoxCommand::redo() { Helper::imageCanvas()->addItem(m_item); }

AddBBoxCommand::~AddBBoxCommand() {
  if (!m_item->scene()) {
    delete m_item;
  }
}

// SizeChangeBBoxCommand
SizeChangeBBoxCommand::SizeChangeBBoxCommand(const QRectF &oldRect,
                                             const QRectF &newRect,
                                             BoundingBoxItem *item,
                                             QUndoCommand *parent)
    : QUndoCommand("SizeChangeBBox", parent), m_oldRect(oldRect),
      m_newRect(newRect), m_item(item) {}

void SizeChangeBBoxCommand::undo() { m_item->setRect(m_oldRect); }

void SizeChangeBBoxCommand::redo() { m_item->setRect(m_newRect); }

// OccludedChangeBBoxCommand
OccludedChangeBBoxCommand::OccludedChangeBBoxCommand(const bool oldOccluded,
                                                     const bool newOccluded,
                                                     BoundingBoxItem *item,
                                                     QUndoCommand *parent)
    : QUndoCommand(getBoolPropertyChangeText("OccludedChange", oldOccluded),
                   parent),
      m_oldOccluded(oldOccluded), m_newOccluded(newOccluded), m_item(item) {}

void OccludedChangeBBoxCommand::undo() { m_item->setOccluded(m_oldOccluded); }

void OccludedChangeBBoxCommand::redo() { m_item->setOccluded(m_newOccluded); }

// bool OccludedChangeBBoxCommand::mergeWith(const QUndoCommand *command) {
//     if (command->id()!=id())
//         return false;

//    const auto ptr = static_cast<const OccludedChangeBBoxCommand *>(command);
//    m_oldOccluded = ptr->m_oldOccluded;
//    m_newOccluded = ptr->m_newOccluded;
//    updateCommandText();
//    return true;
//}

// TruncatedChangeBBoxCommand
TruncatedChangeBBoxCommand::TruncatedChangeBBoxCommand(const bool oldTruncated,
                                                       const bool newTruncated,
                                                       BoundingBoxItem *item,
                                                       QUndoCommand *parent)
    : QUndoCommand(getBoolPropertyChangeText("TruncatedChange", oldTruncated),
                   parent),
      m_oldTruncated(oldTruncated), m_newTruncated(newTruncated), m_item(item) {
}

void TruncatedChangeBBoxCommand::undo() {
  m_item->setTruncated(m_oldTruncated);
}

void TruncatedChangeBBoxCommand::redo() {
  m_item->setTruncated(m_newTruncated);
}

// CrowdedChangeBBoxCommand
CrowdedChangeBBoxCommand::CrowdedChangeBBoxCommand(const bool oldCrowded,
                                                   const bool newCrowded,
                                                   BoundingBoxItem *item,
                                                   QUndoCommand *parent)
    : QUndoCommand(getBoolPropertyChangeText("CrowdedChange", oldCrowded),
                   parent),
      m_oldCrowded(oldCrowded), m_newCrowded(newCrowded), m_item(item) {}

void CrowdedChangeBBoxCommand::undo() { m_item->setCrowded(m_oldCrowded); }

void CrowdedChangeBBoxCommand::redo() { m_item->setCrowded(m_newCrowded); }


/////////////////////////////////////////////////////////////////
////////////////// PolygonItem //////////////////////////////////
/////////////////////////////////////////////////////////////////

// AddBBoxCommand
AddCircleCommand::AddCircleCommand(const QPointF &center, qreal radius, const QString &label, bool ready,
                                   QUndoCommand *parent)
    : QUndoCommand("AddCircle", parent) {
  m_item = new CircleItem(center, radius, label, nullptr, ready);
}

void AddCircleCommand::undo() {
  Helper::imageCanvas()->removeItem(m_item);
}

void AddCircleCommand::redo() {
  Helper::imageCanvas()->addItem(m_item);
}

AddCircleCommand::~AddCircleCommand() {
  if (!m_item->scene()) {
    delete m_item;
  }
}

// RadiusChangeCircleCommand
RadiusChangeCircleCommand::RadiusChangeCircleCommand(const QRectF oldRect,
                                                     const QRectF &newRect,
                                                     CircleItem *item,
                                                     QUndoCommand *parent)
    :QUndoCommand("RadiusChangeCircle", parent),
    m_oldRect{oldRect}, m_newRect{newRect}, m_item{item}{}

void RadiusChangeCircleCommand::undo() {
  m_item->setRect(m_oldRect);
}

void RadiusChangeCircleCommand::redo() {
  m_item->setRect(m_newRect);
}

/////////////////////////////////////////////////////////////////
////////////////// PolygonItem //////////////////////////////////
/////////////////////////////////////////////////////////////////
// AddPolygonCommand
AddPolygonCommand::AddPolygonCommand(const QPolygonF &poly,
                                     const QString &label, bool ready,
                                     QUndoCommand *parent)
    : QUndoCommand("AddPolygon", parent) {
  m_item = new PolygonItem(poly, label, nullptr, ready);
}

void AddPolygonCommand::undo() { Helper::imageCanvas()->removeItem(m_item); }

void AddPolygonCommand::redo() { Helper::imageCanvas()->addItem(m_item); }

AddPolygonCommand::~AddPolygonCommand() {
  if (!m_item->scene()) {
    delete m_item;
  }
}

// ChangePolygonCommand
ChangePolygonCommand::ChangePolygonCommand(const QPolygonF &oldPoly,
                                           const QPolygonF &newPoly,
                                           PolygonItem *item,
                                           QUndoCommand *parent)
    : QUndoCommand("ChangePolygon", parent), m_newPoly(newPoly),
      m_oldPoly(oldPoly), m_item(item) {}

void ChangePolygonCommand::undo() { m_item->setPolygon(m_oldPoly); }

void ChangePolygonCommand::redo() { m_item->setPolygon(m_newPoly); }

/////////////////////////////////////////////////////////////////
//////////////////  LineStrip  //////////////////////////////////
/////////////////////////////////////////////////////////////////
// AddPolygonCommand
AddLineStripCommand::AddLineStripCommand(const QPolygonF &poly,
                                         const QString &label, bool ready,
                                         QUndoCommand *parent)
    : QUndoCommand("AddLineStrip", parent) {
  m_item = new PolygonItem(poly, label, nullptr, ready, false);
}

void AddLineStripCommand::undo() { Helper::imageCanvas()->removeItem(m_item); }

void AddLineStripCommand::redo() { Helper::imageCanvas()->addItem(m_item); }

AddLineStripCommand::~AddLineStripCommand() {
  if (!m_item->scene()) {
    delete m_item;
  }
}

// MovePolygonCommand
ChangeLineStripCommand::ChangeLineStripCommand(const QPolygonF &oldPoly,
                                               const QPolygonF &newPoly,
                                               PolygonItem *item,
                                               QUndoCommand *parent)
    : QUndoCommand("ChangeLineStrip", parent), m_newPoly(newPoly),
      m_oldPoly(oldPoly), m_item(item) {}

void ChangeLineStripCommand::undo() { m_item->setPolygon(m_oldPoly); }

void ChangeLineStripCommand::redo() { m_item->setPolygon(m_newPoly); }

/////////////////////////////////////////////////////////////////
///////////////////  PointItem //////////////////////////////////
/////////////////////////////////////////////////////////////////
// AddPointCommand
AddPointCommand::AddPointCommand(const QPointF &pos, const QString &label,
                                 bool ready, QUndoCommand *parent)
    : QUndoCommand("AddPoint", parent) {
  m_item = new PointItem(pos, label, nullptr, ready);
}

AddPointCommand::~AddPointCommand() {
  if (!m_item->scene()) {
    delete m_item;
  }
}

void AddPointCommand::undo() { Helper::imageCanvas()->removeItem(m_item); }

void AddPointCommand::redo() { Helper::imageCanvas()->addItem(m_item); }

/////////////////////////////////////////////////////////////////
///////////////////// LineItem //////////////////////////////////
/////////////////////////////////////////////////////////////////
// AddLineCommand
AddLineCommand::AddLineCommand(const QPointF &p1, const QPointF &p2,
                               const QString &label, bool ready,
                               QUndoCommand *parent)
    : QUndoCommand("AddLine", parent) {
  m_item = new LineItem(p1, p2, label, nullptr, ready);
}

AddLineCommand::~AddLineCommand() {
  if (!m_item->scene()) {
    delete m_item;
  }
}

void AddLineCommand::undo() { Helper::imageCanvas()->removeItem(m_item); }

void AddLineCommand::redo() { Helper::imageCanvas()->addItem(m_item); }

ChangeLineSizeCommand::ChangeLineSizeCommand(const QLineF &oldLine,
                                             const QLineF &newLine,
                                             LineItem *item,
                                             QUndoCommand *parent)
    : QUndoCommand("ChangeLine", parent), m_item(item), m_oldLine(oldLine),
      m_newLine(newLine) {}

void ChangeLineSizeCommand::undo() { m_item->setLine(m_oldLine); }

void ChangeLineSizeCommand::redo() { m_item->setLine(m_newLine); }

/////////////////////////////////////////////////////////////////
////////////////////////// commond //////////////////////////////
/////////////////////////////////////////////////////////////////
// MoveItemCommand
MoveItemCommand::MoveItemCommand(const QPointF &oldPos, const QPointF &newPos,
                                 QGraphicsItem *item, QUndoCommand *parent)
    : QUndoCommand(parent), m_oldPos(oldPos), m_newPos(newPos), m_item(item) {
  switch (item->type()) {
  case Helper::kBBox:
    setText("MoveBBox");
    break;
  case Helper::kLine:
    setText("MoveLine");
    break;
  case Helper::kPoint:
    setText("MovePoint");
    break;
  case Helper::kPolygon:
    setText("MovePolygon");
    break;
  case Helper::kLineStrip:
    setText("MoveLineStrip");
    break;
  case Helper::kCircle:
    setText("MoveCircle");
    break;
  }
}

void MoveItemCommand::undo() {
  m_item->setPos(m_oldPos);
}

void MoveItemCommand::redo() {
  m_item->setPos(m_newPos); }

// RemoveItemCommand
RemoveItemCommand::RemoveItemCommand(QGraphicsItem *item, QUndoCommand *parent)
    : QUndoCommand(parent), m_item(item) {
  switch (m_item->type()) {
  case Helper::kBBox:
    setText("RemoveBBox");
    break;
  case Helper::kLine:
    setText("RemoveLine");
    break;
  case Helper::kPoint:
    setText("RemovePoint");
    break;
  case Helper::kPolygon:
    setText("RemovePolygon");
    break;
  case Helper::kCircle:
    setText("RemoveCircle");
    break;
  case Helper::kLineStrip:
    setText("RemoveLineStrip");
    break;
  }
}

void RemoveItemCommand::undo() {
  Helper::imageCanvas()->addItem(m_item);
}

void RemoveItemCommand::redo() { Helper::imageCanvas()->removeItem(m_item); }

// RemoveItemCommand
ChangeLabelCommand::ChangeLabelCommand(const QString &oldLabel,
                                       const QString &newLabel,
                                       QGraphicsItem *item,
                                       QUndoCommand *parent)
    : QUndoCommand(parent), m_oldLb(oldLabel), m_newLb(newLabel), m_item(item) {
  switch (m_item->type()) {
  case Helper::kBBox:
    setText("ChangeLabelBBox");
    break;
  case Helper::kLine:
    setText("ChangeLabelLine");
    break;
  case Helper::kPoint:
    setText("ChangeLabelPoint");
    break;
  case Helper::kPolygon:
    setText("ChangeLabelPolygon");
    break;
  case Helper::kLineStrip:
    setText("ChangeLabelLineStrip");
    break;
  case Helper::kCircle:
    setText("ChangeLabelCircle");
    break;
  }
}

void ChangeLabelCommand::undo() { setLabel(m_oldLb); }

void ChangeLabelCommand::redo() { setLabel(m_newLb); }

void ChangeLabelCommand::setLabel(const QString &lb) {
    dynamic_cast<CustomItem *>(m_item)->setLabel(lb);
}
