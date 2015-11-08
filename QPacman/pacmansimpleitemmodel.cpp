/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmansimpleitemmodel.h"
#include <QTreeView>

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

PacmanSimpleItemModel::PacmanSimpleItemModel(QTreeView *parent) : QAbstractItemModel(parent), icon(":/pics/package.png") {
    m_parent = parent;
}

QVariant PacmanSimpleItemModel::data(const QModelIndex & index, int role) const {
    switch (role) {
        case Qt::DisplayRole:
        if (index.column() == 0) return QVariant(rows[index.row()].name);
            else if (index.column() == 2) return QVariant(rows[index.row()].version);
            else if (index.column() == 1) return QVariant(rows[index.row()].desc);
            else return QVariant();
        case Qt::DecorationRole:
            if (index.column() == 0) return QVariant(icon);
            break;
        case Qt::SizeHintRole:
            {
                QFont font = ((QWidget *)((QObject *)this)->parent())->font();
                QFontMetrics fm(font);
                font.setBold(true);
                QFontMetrics fmb(font);
                QSize size(fm.width(data(index,Qt::DisplayRole).toString())+6,fm.height()+4);
                if (index.column() == 0) {
                    size.setWidth(fmb.width(data(index,Qt::DisplayRole).toString())+m_parent->iconSize().width()+8);
                    size.setHeight(max(fmb.height()+4,m_parent->iconSize().height()));
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
        case 0:
            return QVariant(tr("Name of package"));
        case 1:
            return QVariant(tr("Description"));
        case 2:
            return QVariant(tr("Version"));
        default:
            break;
    }

    return QVariant();
}

QModelIndex PacmanSimpleItemModel::index(int row, int column, const QModelIndex & /*parent*/) const {
    return createIndex(row,column);
}

void PacmanSimpleItemModel::addRow(const PacmanEntry & item) {
    rows.append(item);
}

QModelIndex PacmanSimpleItemModel::parent(const QModelIndex & /*index*/) const {
    return QModelIndex();
}

int PacmanSimpleItemModel::rowCount(const QModelIndex & parent) const {
    return parent.isValid()?0:rows.count();
}

PacmanEntry & PacmanSimpleItemModel::row(const QModelIndex & index) {
    return rows[index.row()];
}

PacmanEntry PacmanSimpleItemModel::row(const QModelIndex & index) const {
    int idx = index.row();
    if ((idx >= 0) && (idx < rows.count())) return rows[idx];
    return PacmanEntry();
}
