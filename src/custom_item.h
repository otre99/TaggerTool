#ifndef CUSTOM_ITEM_H
#define CUSTOM_ITEM_H
#include <QString>

class CustomItem {
 public:
  virtual void setLabel(const QString &lb) { m_label = lb; };
  virtual QString label() const { return m_label; };
  virtual void setLocked(bool what) { m_moveEnable = !what; };
  virtual void setShowLabel(bool show) {}
  virtual bool isLocked() const {return !m_moveEnable;}


 protected:
  QString m_label;
  bool m_moveEnable;
  bool m_showLabel{true};
  int m_labelLen;
};

#endif  // CUSTOM_ITEM_H
