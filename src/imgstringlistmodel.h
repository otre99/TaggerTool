#ifndef MYSTRINGLISTMODEL_H
#define MYSTRINGLISTMODEL_H

#include <QObject>
#include <QStringListModel>
#include <QVariant>

class ImgStringListModel : public QStringListModel {
  Q_OBJECT
public:
  ImgStringListModel(QObject *parent = nullptr);
  QString originalText(const QModelIndex &index);
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
};

#endif // MYSTRINGLISTMODEL_H
