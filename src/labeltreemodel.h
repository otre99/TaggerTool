#ifndef LABELTREEMODEL_H
#define LABELTREEMODEL_H

#include <QAbstractItemModel>
#include <QColor>
#include <QList>

#include "annimgmanager.h"
#include "utils.h"

class TreeNode {
  QString m_label{};
  QColor m_color{};
  Qt::CheckState m_checked;
  Helper::CustomItemType m_annType{};
  TreeNode *m_parent{};
  QList<TreeNode *> m_childrens;
  bool m_isRootNode;

 public:
  TreeNode(Helper::CustomItemType annType, const QString &label,
           bool isRootNode = false);
  ~TreeNode();
  Helper::CustomItemType annType() const;
  QString label() const;
  QColor color() const;
  Qt::CheckState isChecked() const;
  void setChecked(Qt::CheckState checked);
  bool isTypeNode() const;
  int childrenCount() const;
  TreeNode *parent();
  TreeNode *addChild(const QString &label);
  TreeNode *child(int row) const;
  int childRow(const TreeNode *child) const;
  int rowInParent() const;
  bool isRootNode() const;
};

class LabelTreeModel : public QAbstractItemModel {
  Q_OBJECT
 public:
  LabelTreeModel(QObject *parent);
  void addNewLabel(Helper::CustomItemType annType, const QString &label);
  void populateLabelsFromAnnotations(const Annotations &ann);
  bool isEnable(Helper::CustomItemType annType, const QString &label) const;

  QModelIndex index(int row, int column,
                    const QModelIndex &parent = {}) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  int rowCount(const QModelIndex &parent = {}) const override;
  int columnCount(const QModelIndex &parent = {}) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  bool setData(const QModelIndex &idx, const QVariant &value,
               int role) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  // QHash<int, QByteArray> roleNames() const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

 public slots:
  void clear();

 private:
  QList<TreeNode *> m_roots;
  TreeNode *nodeFromIndex(const QModelIndex &idx) const;
  QMap<Helper::CustomItemType, QMap<QString, bool>> m_currentLabels;
  TreeNode *ensureRootNode(Helper::CustomItemType &annType, int *nodeRow);
};

#endif  // LABELTREEMODEL_H
