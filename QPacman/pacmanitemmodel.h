/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanItemModel_H
#define PacmanItemModel_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QStringList>
#include "filtertoolbutton.h"
#include "categorytoolbutton.h"
#include "pacmanentry.h"

class QTreeView;

class PacmanItemModel : public QAbstractItemModel {
    Q_OBJECT
public:
    class ChangeMenuParam {
    public:
        ChangeMenuParam(const QIcon & icon,const QString & text,PacmanEntry::UserChangeStatus status) {
            this->icon = icon;
            this->text = text;
            this->status = status;
        }

        QIcon icon;
        QString text;
        PacmanEntry::UserChangeStatus status;
    };

    PacmanItemModel(QTreeView *parent);
    ~PacmanItemModel();
    bool setData(const QModelIndex & index,const QVariant & value,int role = Qt::EditRole);
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole ) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    void addRow(PacmanEntry * item);
    void chooseRow(const QModelIndex & index,bool sel);
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QList<QModelIndex> updatesList() const;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    void sort();
    QStringList getGroups();
    QStringList getRepos();
    QList<int> filterRecords(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo);
    PacmanEntry & row(const QModelIndex & index);
    QList<ChangeMenuParam> changeStateParamsForMenu(const QModelIndex & index) const;
    PacmanEntry row(const QModelIndex & index) const;
    QModelIndex firstFoundIndexByPackageName(const QString & package) const;
    QModelIndex installedIndexByPackageName(const QString & package) const;
    QModelIndex indexByPackageNameVersion(const QString & name,const QString & ver) const;
    QModelIndex installedProviderIndex(const QString & provider) const;
    QModelIndex indexByEntry(const PacmanEntry & entry) const;
    QList<PacmanEntry::InstalledStatus> findPackagesInstalledStatus(const QStringList & packages) const;
    void setFiles(const QString & package,const QStringList & files);
    QList<QModelIndex> indexesCanReplaceInstalled() const;
    bool removeRows(int row,int count,const QModelIndex & parent = QModelIndex());

protected:
    QModelIndex parent(const QModelIndex & index) const;
    virtual QVariant headerData(int section,Qt::Orientation orientation,int role = Qt::DisplayRole) const;
    bool setHeaderData(int section,Qt::Orientation orientation,const QVariant & value,int role = Qt::EditRole);

    QList<PacmanEntry * > rows;
    QTreeView * m_parent;

private:
    QIcon installedIcon;
    QIcon notinstalledIcon;
    QIcon updatedIcon;
    QIcon downloadIcon;
    QIcon removeIcon;
    QIcon reinstallIcon;
    QString unInstallStr;
    QString unInstallAllStr;
    QString reInstallStr;
    QString installStr;
    QStringList m_groups;
    QStringList m_repos;
    QMap<QString,QSet<int> > provides_rows;

    int findLastIndex(int firstindex,int (*compare)(const PacmanEntry * item1, const PacmanEntry * item2)) const;
    int findFirstIndex(int firstindex,int (*compare)(const PacmanEntry * item1, const PacmanEntry * item2)) const;
    void fix_sort_portion(int firstindex,int lastindex);
    void fix_sort_portion2(int firstindex,int lastindex);
    static bool pacman_model_asc_sort(const PacmanEntry * item1, const PacmanEntry * item2);
    static int pacman_model_asc_compare(const PacmanEntry * item1, const PacmanEntry * item2);
    static bool pacman_model_no_version_asc_sort(const PacmanEntry * item1, const PacmanEntry * item2);
    static int pacman_model_no_version_asc_compare(const PacmanEntry * item1, const PacmanEntry * item2);
    QIcon changeStatusIcon(int index) const;
};

#endif // PacmanItemModel_H
