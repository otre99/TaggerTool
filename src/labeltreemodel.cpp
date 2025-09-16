#include "labeltreemodel.h"

TreeNode::TreeNode(Helper::CustomItemType annType, const QString &label,
                   bool isRootNode)
    : m_annType{annType}, m_label{label}, m_isRootNode{isRootNode} {
  // TODO(initialize color
  m_color = Qt::red;
  m_checked = Qt::Checked;
}

TreeNode::~TreeNode() {
  for (auto &&n : m_childrens) {
    delete n;
  }
}

Helper::CustomItemType TreeNode::annType() const { return m_annType; }

QString TreeNode::label() const { return m_label; }
QColor TreeNode::color() const { return m_color; }
Qt::CheckState TreeNode::isChecked() const { return m_checked; }
void TreeNode::setChecked(Qt::CheckState checked) { m_checked = checked; }
int TreeNode::childrenCount() const { return m_childrens.size(); }
TreeNode *TreeNode::parent() { return m_parent; }
TreeNode *TreeNode::addChild(const QString &label) {
  TreeNode *it = new TreeNode(m_annType, label);
  m_childrens.push_back(it);
  it->m_parent = this;
  return it;
}

TreeNode *TreeNode::child(int row) const { return m_childrens[row]; }
int TreeNode::childRow(const TreeNode *child) const {
  return m_childrens.indexOf(child);
}
int TreeNode::rowInParent() const {
  return m_parent ? m_parent->childRow(this) : -1;
}

bool TreeNode::isRootNode() const { return m_isRootNode; }

/////////////////////////////////////////////////
/// \brief LabelTreeModel
/////////////////////////////////////////////////

LabelTreeModel::LabelTreeModel(QObject *parent) : QAbstractItemModel(parent) {}

void LabelTreeModel::addNewLabel(Helper::CustomItemType annType,
                                 const QString &label) {
  int node_row;
  TreeNode *rootNode = ensureRootNode(annType, &node_row);
  if (m_currentLabels[annType].contains(label)) {
    return;
  }
  beginInsertRows(createIndex(node_row, 0, rootNode), rootNode->childrenCount(),
                  rootNode->childrenCount());
  rootNode->addChild(label);
  endInsertRows();
  m_currentLabels[annType].insert(label, true);
}

void LabelTreeModel::populateLabelsFromAnnotations(const Annotations &ann) {
  qDebug() << "----------------";
  for (auto &o : ann.bboxes) addNewLabel(Helper::kBBox, o.getLabel());
  for (auto &o : ann.polygons) addNewLabel(Helper::kPolygon, o.getLabel());
  for (auto &o : ann.points) addNewLabel(Helper::kPoint, o.getLabel());
  for (auto &o : ann.line_strips) addNewLabel(Helper::kLineStrip, o.getLabel());
  for (auto &o : ann.circles) addNewLabel(Helper::kCircle, o.getLabel());
  for (auto &o : ann.lines) addNewLabel(Helper::kLine, o.getLabel());
}

bool LabelTreeModel::isEnable(Helper::CustomItemType annType,
                              const QString &label) const {
  return m_currentLabels[annType][label];
}

QModelIndex LabelTreeModel::index(int row, int column,
                                  const QModelIndex &parentIdx) const {
  if (column != 0 || row < 0) return {};

  TreeNode *parentNode = nodeFromIndex(parentIdx);  // invalid -> m_root

  if (parentNode) {
    if (row >= parentNode->childrenCount()) return {};
    TreeNode *child = parentNode->child(row);
    return createIndex(row, column, child);
  }

  if (row >= m_roots.size()) return {};
  TreeNode *child = m_roots.at(row);
  return createIndex(row, column, child);
}

QModelIndex LabelTreeModel::parent(const QModelIndex &childIdx) const {
  if (!childIdx.isValid()) return {};

  TreeNode *n = static_cast<TreeNode *>(childIdx.internalPointer());
  if (!n) return {};

  TreeNode *p = n->parent();
  if (!p) return {};

  int row = p->rowInParent();
  if (row < 0) {
    return createIndex(m_roots.indexOf(p), 0, p);
  }
  return createIndex(row, 0, p);
}

int LabelTreeModel::rowCount(const QModelIndex &parentIdx) const {
  if (parentIdx.column() > 0) return 0;
  TreeNode *parentNode = nodeFromIndex(parentIdx);
  if (parentNode) {
    return parentNode->childrenCount();
  }
  return m_roots.size();
}

int LabelTreeModel::columnCount(const QModelIndex &parentIdx) const {
  if (parentIdx.column() > 0) return 0;
  return 1;
}

QVariant LabelTreeModel::data(const QModelIndex &idx, int role) const {
  if (!idx.isValid()) return {};
  auto *n = static_cast<TreeNode *>(idx.internalPointer());

  switch (role) {
    case Qt::DisplayRole:
      return n->label();
    case Qt::CheckStateRole:
      if (n->isRootNode())
        return {};
      else
        return n->isChecked();
      break;
    default:
      return {};
  }
}

bool LabelTreeModel::setData(const QModelIndex &idx, const QVariant &value,
                             int role) {
  if (!idx.isValid()) return false;
  auto *n = static_cast<TreeNode *>(idx.internalPointer());
  if (n->isRootNode()) return false;

  if (role == Qt::CheckStateRole && !n->isRootNode()) {
    Qt::CheckState checked = static_cast<Qt::CheckState>(value.toInt());
    if (checked == n->isChecked()) return true;
    n->setChecked(checked);
    emit dataChanged(idx, idx, {role});
    m_currentLabels[n->annType()][n->label()] = checked == Qt::Checked;
    return true;
  }
  return false;
}

Qt::ItemFlags LabelTreeModel::flags(const QModelIndex &idx) const {
  if (!idx.isValid()) return Qt::NoItemFlags;
  auto *n = static_cast<TreeNode *>(idx.internalPointer());

  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if (!n->isRootNode()) f |= Qt::ItemIsUserCheckable;
  return f;
}

QVariant LabelTreeModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
  switch (role) {
    case Qt::DisplayRole:
      return "Annotation Labels";
      break;
    default:
      return {};
      break;
  }
}

void LabelTreeModel::clear() {
  beginResetModel();

  for (TreeNode *n : std::as_const(m_roots)) {
    delete n;
  }
  m_roots.clear();
  m_currentLabels.clear();
  endResetModel();
}

////////////////////////////////////////////////////////////
///////////////// PRIVATE FUNTIONS /////////////////////////
////////////////////////////////////////////////////////////
TreeNode *LabelTreeModel::nodeFromIndex(const QModelIndex &idx) const {
  return idx.isValid() ? static_cast<TreeNode *>(idx.internalPointer())
                       : nullptr;
}

TreeNode *LabelTreeModel::ensureRootNode(Helper::CustomItemType &annType,
                                         int *nodeRow) {
  if (m_currentLabels.contains(annType)) {
    int row_index = 0;
    for (TreeNode *node_ptr : std::as_const(m_roots)) {
      if (node_ptr->annType() == annType) {
        *nodeRow = row_index;
        return node_ptr;
      }
      row_index += 1;
    }
  }

  QString rootLabel;
  switch (annType) {
    case Helper::kBBox:
      rootLabel = QStringLiteral("BBoxes");
      break;
    case Helper::kPolygon:
      rootLabel = QStringLiteral("Polygons");
      break;
    case Helper::kPoint:
      rootLabel = QStringLiteral("Points");
      break;
    case Helper::kLine:
      rootLabel = QStringLiteral("Lines");
      break;
    case Helper::kLineStrip:
      rootLabel = QStringLiteral("LineStrips");
      break;
    case Helper::kCircle:
      rootLabel = QStringLiteral("Circles");
      break;
    case Helper::kNoItem:
      rootLabel = QStringLiteral("----");
  }

  beginInsertRows(QModelIndex(), m_roots.size(), m_roots.size());
  *nodeRow = m_roots.size();
  TreeNode *node = new TreeNode(annType, rootLabel, true);
  m_roots.append(node);
  endInsertRows();
  return node;
}
