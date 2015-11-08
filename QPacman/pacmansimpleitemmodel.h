/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANSIMPLEITEMMODEL_H
#define PACMANSIMPLEITEMMODEL_H

#include <QAbstractItemModel>
#include "pacmanentry.h"

class QTreeView;

class PacmanSimpleItemModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit PacmanSimpleItemModel(QTreeView *parent = 0);
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole ) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    void addRow(const PacmanEntry & item);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    PacmanEntry row(const QModelIndex & index) const;
    PacmanEntry & row(const QModelIndex & index);

protected:
    QVariant headerData(int section,Qt::Orientation orientation,int role = Qt::DisplayRole) const;
    QModelIndex parent(const QModelIndex & index) const;

private:
    QIcon icon;
    QList<PacmanEntry> rows;
    QTreeView * m_parent;
};

#endif // PACMANSIMPLEITEMMODEL_H
