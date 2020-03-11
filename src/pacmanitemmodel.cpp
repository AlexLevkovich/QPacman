/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanitemmodel.h"
#include <QFontMetrics>
#include <QWidget>
#include <QTreeView>
#include <stdio.h>
#include "static.h"
#include "pacmanview.h"
#include "libalpm.h"
#include "themeicons.h"
#include "optionswidget.h"

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

PacmanItemModel::PacmanItemModel(PacmanView *parent) : QAbstractItemModel(parent),
                                                       installedIcon(ThemeIcons::get(ThemeIcons::PKG_INSTALLED_MARK)),
                                                       notinstalledIcon(ThemeIcons::get(ThemeIcons::PKG_NONINSTALLED_MARK)),
                                                       updatedIcon(ThemeIcons::get(ThemeIcons::UPDATE_ITEM)) {
    m_parent = (PacmanView *)parent;
}

PacmanItemModel::~PacmanItemModel() {}

QVariant PacmanItemModel::headerData(int section,Qt::Orientation orientation,int role) const {
    if (orientation != Qt::Horizontal) return QVariant();
    if (role != Qt::DisplayRole) return QVariant();

    switch (section) {
        case Name:
            return QVariant(tr("Name of package"));
        case Description:
            return QVariant(tr("Description"));
        case Version:
            return QVariant(tr("Version"));
        case Repository:
            return QVariant(tr("Repository"));
        case Action:
            return QVariant(tr("Operation"));
        default:
            break;
    }

    return QVariant();
}

bool PacmanItemModel::setHeaderData(int /*section*/,Qt::Orientation /*orientation*/,const QVariant & /*value*/,int /*role*/) {
    return false;
}

int PacmanItemModel::columnCount(const QModelIndex & /*parent*/) const {
    return (getuid() != 0)?4:5;
}

QVariant PacmanItemModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid()) return QVariant();
    if (index.row() >= rowCount()) return QVariant();

    const QVector<AlpmPackage *> & rows = packages();

    switch (role) {
        case Qt::DisplayRole:
            if (index.column() == Name) return QVariant(rows[index.row()]->name());
            else if (index.column() == Version) return QVariant(rows[index.row()]->version());
            else if (index.column() == Repository) return QVariant(rows[index.row()]->repo());
            else if (index.column() == Description) return QVariant(rows[index.row()]->description());
            return OptionsWidget::statusText(rows[index.row()]->changeStatus());
        case Qt::DecorationRole:
            if (index.column() == Name) return rows[index.row()]->isChosen()?OptionsWidget::statusIcon(rows[index.row()]->changeStatus()):(QVariant(rows[index.row()]->isInstalled()?installedIcon:(rows[index.row()]->isUpdate()?updatedIcon:notinstalledIcon)));
            if (index.column() == Action) return OptionsWidget::statusIcon(rows[index.row()]->changeStatus());
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
                    size.setHeight(max(fmb.height()+4,m_parent->iconSize().height()));
                }
                return QVariant(size);
            }
        case Qt::ToolTipRole:
            if (rows[index.row()]->isChosen()) return OptionsWidget::statusTextHint(rows[index.row()]->changeStatus());
        default:
            break;
    }

    return QVariant();
}

bool PacmanItemModel::setData(const QModelIndex & index,const QVariant & value,int role) {
    QVector<AlpmPackage *> & rows = (QVector<AlpmPackage *> &)packages();
    if ((role == Qt::DisplayRole) && (index.column() == Action)) {
        rows[index.row()]->setChangeStatus((AlpmPackage::UserChangeStatus)value.toInt());
        return true;
    }

    return false;
}

QModelIndex PacmanItemModel::index(int row, int column, const QModelIndex & /*parent*/) const {
    return createIndex(row,column);
}

QModelIndex PacmanItemModel::parent(const QModelIndex & /*index*/) const {
    return QModelIndex();
}

int PacmanItemModel::rowCount(const QModelIndex & parent) const {
    return parent.isValid()?0:packages().count();
}

bool PacmanItemModel::removeRows(int row,int count,const QModelIndex &parent) {
    if (count <= 0) return false;
    beginRemoveRows(parent,row,row+(count -1));
    endRemoveRows();
    return true;
}

void PacmanItemModel::refreshRows() {
    const QVector<AlpmPackage *> & pkgs = packages();
    for (int i=0;i<pkgs.count();i++) {
        QList<AlpmPackage::Dependence> provides = pkgs[i]->provides();
        if (provides.size() > 0) {
            for (int j=0;j<provides.size();j++) {
                m_provides[provides.at(j)].append(i);
            }
        }
        m_groups.append(pkgs[i]->groups());
    }
    m_groups.removeDuplicates();

    beginInsertRows(QModelIndex(),0,packages().count()-1);
    endInsertRows();
}

void PacmanItemModel::chooseRow(const QModelIndex & index,bool sel) {
    QVector<AlpmPackage *> & rows = (QVector<AlpmPackage *> &)packages();
    rows[index.row()]->m_isChosen = sel;
}

QList<int> PacmanItemModel::filterRecords(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo) {
    QList<int> list;

    QVector<AlpmPackage *> packageNames;
    if (cItemId == CategoryToolButton::IS_FILE_NAME) {
        packageNames = Alpm::instance()->findByFileName(text);
    }

    const QVector<AlpmPackage *> & rows = packages();

    for (int i=0;i<rows.size();i++) {
        bool text_is_ok = false;
        if (text.isEmpty() && packageNames.isEmpty() && cItemId == CategoryToolButton::IS_FILE_NAME) text_is_ok = true;
        else text_is_ok = (cItemId == CategoryToolButton::IS_FILE_NAME)?(packageNames.indexOf(rows[i]) >= 0):rows[i]->containsText(text,cItemId);

        if (group.isEmpty()) {
            if (((fItemId == FilterToolButton::IS_INSTALLED) && !rows[i]->isInstalled()) || !text_is_ok || ((rows[i]->repo() != repo) && !repo.isEmpty() && (rows[i]->repo() != Static::RepoAll_Str))) list.append(i);
            else if (((fItemId == FilterToolButton::IS_NONINSTALLED) && rows[i]->isInstalled()) || !text_is_ok || ((rows[i]->repo() != repo) && !repo.isEmpty() && (rows[i]->repo() != Static::RepoAll_Str))) list.append(i);
            else if (((fItemId == FilterToolButton::IS_NEEDUPDATE) && !rows[i]->isUpdate()) || !text_is_ok || ((rows[i]->repo() != repo) && !repo.isEmpty() && (rows[i]->repo() != Static::RepoAll_Str))) list.append(i);
            else if (((fItemId == FilterToolButton::IS_ORPHANED) && !rows[i]->isOrphaned()) || !text_is_ok || ((rows[i]->repo() != repo) && !repo.isEmpty() && (rows[i]->repo() != Static::RepoAll_Str))) list.append(i);
            else if (((fItemId == FilterToolButton::IS_MARKED) && !rows[i]->isChosen()) || !text_is_ok || ((rows[i]->repo() != repo) && !repo.isEmpty() && (rows[i]->repo() != Static::RepoAll_Str))) list.append(i);
        }
        else{
            if (!rows[i]->ownedByGroup(group) || !text_is_ok || ((rows[i]->repo() != repo) && !repo.isEmpty() && (rows[i]->repo() != Static::RepoAll_Str))) list.append(i);
        }
    }

    return list;
}

AlpmPackage * PacmanItemModel::row(const QModelIndex & index) {
    const QVector<AlpmPackage *> & rows = packages();
    if (index.row() >= rows.count()) return NULL;
    return rows[index.row()];
}

QModelIndex PacmanItemModel::firstFoundUninstalledIndexByPackageName(const QString & package) const {
    QVector<qint64> indexes = findCacheIndexesByPackageName(package);
    const QVector<AlpmPackage *> & rows = packages();

    for (int i=0;i<indexes.count();i++) {
        if (!rows[indexes[i]]->isInstalled()) return index(indexes[i],Name);
    }

    return QModelIndex();
}

QModelIndex PacmanItemModel::firstFoundIndexByDep(const AlpmPackage::Dependence & pkg) const {
    QVector<qint64> indexes = findCacheIndexesByPackageName(pkg.name());
    if (indexes.count() <= 0) {
        indexes = findCacheIndexesByPackageNameProvides(pkg);
        if (indexes.count() <= 0) return QModelIndex();
    }

    return index(indexes[0],Name);
}

QModelIndex PacmanItemModel::installedIndexByPackageName(const QString & package) const {
    QVector<qint64> indexes = findCacheIndexesByPackageName(package);
    if (indexes.count() <= 0) return QModelIndex();

    const QVector<AlpmPackage *> & rows = packages();
    for (int i=0;i<indexes.count();i++) {
        if (rows[indexes[i]]->isInstalled()) return index(indexes[i],Name);
    }

    return QModelIndex();
}

QModelIndex PacmanItemModel::indexByPackageNameVersion(const QString & name,const QString & ver) const {
    QVector<qint64> indexes = findCacheIndexesByPackageNameVersion(name,ver);
    if (indexes.count() <= 0) return QModelIndex();

    return index(indexes[0],Name);
}

QModelIndex PacmanItemModel::indexByEntry(AlpmPackage * entry) const {
    return indexByPackageNameVersion(entry->name(),entry->version());
}

const QVector<AlpmPackage *> & PacmanItemModel::packages() const {
    return Alpm::instance()->lastQueriedPackages();
}

const QStringList & PacmanItemModel::groups() const {
    return m_groups;
}

const QMap<AlpmPackage::Dependence,QVector<qint64> > & PacmanItemModel::provides() const {
    return m_provides;
}
