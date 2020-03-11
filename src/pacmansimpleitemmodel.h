/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANSIMPLEITEMMODEL_H
#define PACMANSIMPLEITEMMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include "alpmpackage.h"

class QTreeView;

class PacmanSimpleItemModel : public QAbstractItemModel {
    Q_OBJECT
public:
    PacmanSimpleItemModel(QTreeView *parent = 0);
    ~PacmanSimpleItemModel();
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole ) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    void addRow(AlpmPackage * item);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    AlpmPackage * row(const QModelIndex & index);
    QList<AlpmPackage *> allrows() const { return rows; }

protected:
    QVariant headerData(int section,Qt::Orientation orientation,int role = Qt::DisplayRole) const;
    QModelIndex parent(const QModelIndex & index) const;

private:
    enum Columns {
        Name = 0,
        Description,
        Version
    };

    QIcon icon;
    QList<AlpmPackage *> rows;
    QTreeView * m_parent;
};

#endif // PACMANSIMPLEITEMMODEL_H
