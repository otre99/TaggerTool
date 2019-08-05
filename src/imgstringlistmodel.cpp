#include "imgstringlistmodel.h"

ImgStringListModel::ImgStringListModel(QObject *parent)
    : QStringListModel (parent)
{

}

QString ImgStringListModel::GetOriginalText(const QModelIndex &index)
{
    return QStringListModel::data(index).toString();
}

QVariant ImgStringListModel::data(const QModelIndex &index, int role) const
{
    QVariant r = QStringListModel::data(index, role);
    if ( r.isValid() && role == Qt::DisplayRole )
        return "[ "+QString::number( index.row() )+"] " + r.toString();
    return r;
}

