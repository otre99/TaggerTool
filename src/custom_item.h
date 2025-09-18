#ifndef CUSTOM_ITEM_H
#define CUSTOM_ITEM_H
#include <QPoint>
#include <QString>

class QAbstractGraphicsShapeItem;
class QGraphicsItem;
class ImageCanvas;

class CustomItem {
 public:
  virtual void setLabel(const QString &lb) = 0;
  virtual void setDescription(const QString &dsc) { m_description = dsc; }
  virtual QString label() const { return m_label; }
  virtual QString description() const { return m_description; }
  virtual void setLocked(bool what) = 0;
  virtual void helperParametersChanged() = 0;
  virtual void setShowLabel(bool show) {}
  virtual bool isLocked() const { return !m_editEnable; }
  virtual void showEditDialog(QGraphicsItem *item, const QPoint screenPos);

 protected:
  QString m_label;
  QString m_description;
  bool m_editEnable{false};
  int m_labelLen;
  int m_labelHeight;
  ImageCanvas *m_canvas;

  void __setLabel(QAbstractGraphicsShapeItem *item, QString label);
  void __calculateLabelSize(const QString &label);
  void __setLocked(QGraphicsItem *item, bool lk);
  void __swapStackOrder(QGraphicsItem *item, const QList<QGraphicsItem *> &l);
};

#endif  // CUSTOM_ITEM_H
