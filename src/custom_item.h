#ifndef CUSTOM_ITEM_H
#define CUSTOM_ITEM_H
#include <QString>

class QAbstractGraphicsShapeItem;
class QGraphicsItem;

class CustomItem {
 public:
  virtual void setLabel(const QString &lb) = 0;
  virtual QString label() const { return m_label; };
  virtual void setLocked(bool what) = 0;
  virtual void helperParametersChanged() = 0;
  virtual void setShowLabel(bool show) {}
  virtual bool isLocked() const { return !m_moveEnable; }

 protected:
  QString m_label;
  bool m_moveEnable{false};
  bool m_showLabel{true};
  int m_labelLen;
  int m_labelHeight;

  void __setLabel(QAbstractGraphicsShapeItem *item, QString label);
  void __calculateLabelSize(const QString &label);
  void __setLocked(QGraphicsItem *item, bool lk);
  void __swapStackOrder(QGraphicsItem *item, const QList<QGraphicsItem *> &l);
};

#endif  // CUSTOM_ITEM_H
