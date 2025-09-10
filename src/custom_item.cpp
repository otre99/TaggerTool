#include "custom_item.h"

#include <QAbstractGraphicsShapeItem>
#include <QDebug>
#include <QFontMetrics>
#include <QPen>
#include <QTimer>

#include "editdialog.h"
#include "imagecanvas.h"
#include "undo_cmds.h"
#include "utils.h"

void CustomItem::showEditDialog(QGraphicsItem *item, const QPoint screenPos) {
  EditDialog dlg;
  dlg.setGeometry(QRect{screenPos, dlg.size()});
  dlg.setLabel(m_label);
  dlg.setDescription(m_description);
  if (dlg.exec() == QDialog::Accepted) {
    // ImageCanvas *canvas = dynamic_cast<ImageCanvas *>(item->scene());
    ImageCanvas *canvas = reinterpret_cast<ImageCanvas *>(item->scene());

    if (dlg.removeItem()) {
      emit canvas->deferredRemoveItem(item);
      return;
    }

    if (dlg.label() != m_label) {
      Helper::imageCanvas()->undoStack()->push(
          new ChangeLabelCommand(m_label, dlg.label(), item));
    }

    if (dlg.description() != m_description) {
      Helper::imageCanvas()->undoStack()->push(
          new ChangeDescriptionCommand(m_description, dlg.description(), item));
    }
  }
}

void CustomItem::__setLabel(QAbstractGraphicsShapeItem *item, QString label) {
  m_label = label;
  auto p = item->pen();
  p.setColor(Helper::colorFromLabel(label));
  item->setPen(p);
  __calculateLabelSize(label);
}

void CustomItem::__calculateLabelSize(const QString &label) {
  QFontMetrics fm(Helper::fontLabel());
  m_labelLen = fm.horizontalAdvance(label);
  m_labelHeight = fm.height();
}

void CustomItem::__setLocked(QGraphicsItem *item, bool lk) {
  m_moveEnable = !lk;
  item->setFlag(QGraphicsItem::ItemIsMovable, m_moveEnable);
  item->setFlag(QGraphicsItem::ItemIsSelectable, m_moveEnable);
  item->setFlag(QGraphicsItem::ItemIsFocusable, m_moveEnable);

  if (m_canvas && lk == false) {
    m_canvas->updateMovableItem(this);
  }
}

void CustomItem::__swapStackOrder(QGraphicsItem *item,
                                  const QList<QGraphicsItem *> &l) {
  if (l.count() > 1) {
    for (int i = 1; i < l.count(); ++i) item->stackBefore(l[i]);
    auto *citem = dynamic_cast<CustomItem *>(l[1]);
    if (citem) {
      citem->setLocked(false);
      dynamic_cast<CustomItem *>(item)->setLocked(true);
    }
  }
}
