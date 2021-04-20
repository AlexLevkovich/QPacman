/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmansimpleitemmodel.h"
#include "themeicons.h"
#include "libalpm.h"
#include <QTreeView>

PacmanSimpleItemModel::PacmanSimpleItemModel(QTreeView *parent) : QAbstractItemModel(parent), icon(ThemeIcons::get(ThemeIcons::PKG)) {
    m_parent = parent;
}

QVariant PacmanSimpleItemModel::data(const QModelIndex & index, int role) const {
    switch (role) {
        case Qt::DisplayRole:
        if (index.column() == Name) return QVariant(rows[index.row()].name());
            else if (index.column() == Version) return QVariant(rows[index.row()].version());
            else if (index.column() == Description) return QVariant(rows[index.row()].description());
            else return QVariant();
        case Qt::DecorationRole:
            if (index.column() == Name) return QVariant(icon);
            break;
        case Qt::SizeHintRole:
            {
                QFont font = ((QWidget *)((QObject *)this)->parent())->font();
                QFontMetrics fm(font);
                font.setBold(true);
                QFontMetrics fmb(font);
                QSize size(fm.horizontalAdvance(data(index,Qt::DisplayRole).toString())+6,fm.height()+4);
                if (index.column() == Name) {
                    size.setWidth(fmb.horizontalAdvance(data(index,Qt::DisplayRole).toString())+m_parent->iconSize().width()+8);
                    size.setHeight(qMax(fmb.height()+4,m_parent->iconSize().height()));
                }
                return QVariant(size);
            }
        default:
            break;
    }

    return QVariant();
}

int PacmanSimpleItemModel::columnCount(const QModelIndex & /*parent*/) const {
    return 3;
}

QVariant PacmanSimpleItemModel::headerData(int section,Qt::Orientation orientation,int role) const {
    if (orientation != Qt::Horizontal) return QVariant();
    if (role != Qt::DisplayRole) return QVariant();

    switch (section) {
        case Name:
            return QVariant(tr("Name of package"));
        case Description:
            return QVariant(tr("Description"));
        case Version:
            return QVariant(tr("Version"));
        default:
            break;
    }

    return QVariant();
}

QModelIndex PacmanSimpleItemModel::index(int row, int column, const QModelIndex & /*parent*/) const {
    return createIndex(row,column);
}

void PacmanSimpleItemModel::addRow(const AlpmPackage & item) {
    beginInsertRows(QModelIndex(),rows.count(),rows.count());
    rows.append(item);
    endInsertRows();
}

QModelIndex PacmanSimpleItemModel::parent(const QModelIndex & /*index*/) const {
    return QModelIndex();
}

int PacmanSimpleItemModel::rowCount(const QModelIndex & parent) const {
    return parent.isValid()?0:rows.count();
}

AlpmPackage & PacmanSimpleItemModel::row(const QModelIndex & index) {
    return rows[index.row()];
}

void PacmanSimpleItemModel::forbidRowDeletions() {
    for (AlpmPackage & pkg: rows) pkg.setLocalPackageDeletionAllowed(false);
}
