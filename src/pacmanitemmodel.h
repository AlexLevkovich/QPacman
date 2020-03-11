/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanItemModel_H
#define PacmanItemModel_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QVector>
#include <QStringList>
#include "filtertoolbutton.h"
#include "categorytoolbutton.h"
#include "alpmpackage.h"
#include "alpmdb.h"

class PacmanView;

class PacmanItemModel : public QAbstractItemModel, public AlpmAbstractDB {
    Q_OBJECT
public:
    PacmanItemModel(PacmanView *parent);
    ~PacmanItemModel();
    bool setData(const QModelIndex & index,const QVariant & value,int role = Qt::EditRole);
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole ) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    void refreshRows();
    void chooseRow(const QModelIndex & index,bool sel);
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    QList<int> filterRecords(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo);
    AlpmPackage * row(const QModelIndex & index);
    QModelIndex firstFoundIndexByDep(const AlpmPackage::Dependence & pkg) const;
    QModelIndex firstFoundUninstalledIndexByPackageName(const QString & package) const;
    QModelIndex installedIndexByPackageName(const QString & package) const;
    QModelIndex indexByPackageNameVersion(const QString & name,const QString & ver) const;
    QModelIndex indexByEntry(AlpmPackage * entry) const;
    bool removeRows(int row,int count,const QModelIndex &parent = QModelIndex());

protected:
    QModelIndex parent(const QModelIndex & index) const;
    virtual QVariant headerData(int section,Qt::Orientation orientation,int role = Qt::DisplayRole) const;
    bool setHeaderData(int section,Qt::Orientation orientation,const QVariant & value,int role = Qt::EditRole);

    const QVector<AlpmPackage *> & packages() const;
    const QStringList & groups() const;
    const QMap<AlpmPackage::Dependence,QVector<qint64> > & provides() const;

    PacmanView * m_parent;

private:
    enum Columns {
        Name = 0,
        Description,
        Version,
        Repository,
        Action
    };

    QIcon installedIcon;
    QIcon notinstalledIcon;
    QIcon updatedIcon;
    QStringList m_groups;
    QMap<AlpmPackage::Dependence,QVector<qint64> > m_provides;
};

#endif // PacmanItemModel_H
