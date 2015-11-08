/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanitemmodel.h"
#include <QFontMetrics>
#include <QWidget>
#include <QTreeView>
#include <stdio.h>

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

extern "C" int alpm_pkg_vercmp(const char *a, const char *b);

static void updateText(QFontMetrics & fm,int max_width,QString & str) {
    for(;;) {
        str+=" ";
        if (fm.width(str) >= max_width) break;
    }
}

PacmanItemModel::PacmanItemModel(QTreeView *parent) : QAbstractItemModel(parent),
    installedIcon(":/pics/dialog-ok-apply.png"),notinstalledIcon(":/pics/notinstalled.png"),
    updatedIcon(":/pics/package-update.png"),downloadIcon(":/pics/down.png"),
    removeIcon(":/pics/delete.png"),reinstallIcon(":/pics/2down.png") {

    unInstallStr = QObject::tr("Uninstall");
    installStr = QObject::tr("Install");
    reInstallStr = QObject::tr("Reinstall");

    m_parent = parent;
    QFontMetrics fm(((QWidget *)((QObject *)this)->parent())->font());
    int installWidth = fm.width(installStr);
    int unInstallWidth = fm.width(unInstallStr);
    int maxInstallWidth = max(fm.width(reInstallStr),installWidth);
    maxInstallWidth = max(maxInstallWidth,unInstallWidth);

    updateText(fm,maxInstallWidth,installStr);
    updateText(fm,maxInstallWidth,unInstallStr);
    updateText(fm,maxInstallWidth,reInstallStr);
}

QVariant PacmanItemModel::headerData(int section,Qt::Orientation orientation,int role) const {
    if (orientation != Qt::Horizontal) return QVariant();
    if (role != Qt::DisplayRole) return QVariant();

    switch (section) {
        case 0:
            return QVariant(tr("Name of package"));
        case 1:
            return QVariant(tr("Description"));
        case 2:
            return QVariant(tr("Version"));
        case 3:
            return QVariant(tr("Repository"));
        case 4:
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
    return 5;
}

QList<PacmanItemModel::ChangeMenuParam> PacmanItemModel::changeStateParamsForMenu(const QModelIndex & index) const {
    PacmanEntry::InstalledStatus status = row(index).getStatus();
    if (status == PacmanEntry::NOT_INSTALLED) {
        return QList<ChangeMenuParam>() << ChangeMenuParam(downloadIcon,installStr,PacmanEntry::DO_INSTALL);
    }
    else if ((status != PacmanEntry::NOT_INSTALLED) && (row(index).getRepo() == "aur")) {
        return QList<ChangeMenuParam>() << ChangeMenuParam(removeIcon,unInstallStr,PacmanEntry::DO_UNINSTALL);
    }

    return QList<ChangeMenuParam>() << ChangeMenuParam(removeIcon,unInstallStr,PacmanEntry::DO_UNINSTALL) << ChangeMenuParam(reinstallIcon,reInstallStr,PacmanEntry::DO_REINSTALL);
}

QIcon PacmanItemModel::changeStatusIcon(int index) const {
    PacmanEntry::UserChangeStatus change_status = rows[index].getChangeStatus();
    if (change_status == PacmanEntry::DO_INSTALL) return downloadIcon;
    else if (change_status == PacmanEntry::DO_REINSTALL) return reinstallIcon;

    return removeIcon;
}

QVariant PacmanItemModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid()) return QVariant();
    if (index.row() >= rowCount()) return QVariant();

    switch (role) {
        case Qt::DisplayRole:
            if (index.column() == 0) return QVariant(rows[index.row()].name);
            else if (index.column() == 2) return QVariant(rows[index.row()].version);
            else if (index.column() == 3) return QVariant(rows[index.row()].repo);
            else if (index.column() == 1) return QVariant(rows[index.row()].desc);
            else {
                PacmanEntry::UserChangeStatus change_status = rows[index.row()].getChangeStatus();
                return QVariant((change_status == PacmanEntry::DO_UNINSTALL)?unInstallStr:((change_status == PacmanEntry::DO_INSTALL)?installStr:reInstallStr));
            }
        case Qt::DecorationRole:
            if (index.column() == 0) return rows[index.row()].isChosen()?changeStatusIcon(index.row()):(QVariant(rows[index.row()].isInstalled()?installedIcon:(rows[index.row()].isUpdate()?updatedIcon:notinstalledIcon)));
            if (index.column() == 4) return changeStatusIcon(index.row());
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

bool PacmanItemModel::setData(const QModelIndex & index,const QVariant & value,int role) {
    if ((role == Qt::DisplayRole) && (index.column() == 4)) {
        rows[index.row()].setChangeStatus((PacmanEntry::UserChangeStatus)value.toInt());
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
    return parent.isValid()?0:rows.count();
}

void PacmanItemModel::addRow(const PacmanEntry & item) {
    rows.append(item);
    for (int i=0;i<item.groups.count();i++) {
        m_groups.insert(item.groups[i]);
        m_repos.insert(item.repo);
    }
}

QStringList PacmanItemModel::getGroups() {
    return m_groups.toList();
}

QStringList PacmanItemModel::getRepos() {
    return m_repos.toList();
}

void PacmanItemModel::chooseRow(const QModelIndex & index,bool sel) {
    rows[index.row()].m_isChosen = sel;
}

bool PacmanItemModel::pacman_model_asc_sort(const PacmanEntry & item1, const PacmanEntry & item2) {
    return pacman_model_asc_compare(item1,item2) < 0;
}

bool PacmanItemModel::pacman_model_no_version_asc_sort(const PacmanEntry & item1, const PacmanEntry & item2) {
    return pacman_model_no_version_asc_compare(item1,item2) < 0;
}

int PacmanItemModel::pacman_model_asc_compare(const PacmanEntry & item1, const PacmanEntry & item2) {
    int Ret;
    return (Ret = item1.name.compare(item2.name))?Ret:
                  alpm_pkg_vercmp((const char *)item1.version.toLocal8Bit(),(const char *)item2.version.toLocal8Bit());
}

int PacmanItemModel::pacman_model_no_version_asc_compare(const PacmanEntry & item1, const PacmanEntry & item2) {
    return item1.name.compare(item2.name);
}

int PacmanItemModel::findLastIndex(int bIndex,int (*compare)(const PacmanEntry & item1, const PacmanEntry & item2)) const {
    for (int i=bIndex;i<rows.count();i++) {
        if (compare(rows[bIndex],rows[i])) {
            return i - 1;
        }
    }
    return rows.count()-1;
}

int PacmanItemModel::findFirstIndex(int bIndex,int (*compare)(const PacmanEntry & item1, const PacmanEntry & item2)) const {
    for (int i=bIndex;i>=0;i--) {
        if (compare(rows[bIndex],rows[i])) {
            return i + 1;
        }
    }
    return 0;
}

void PacmanItemModel::fix_sort_portion(int firstindex,int lastindex) {
    if (lastindex > firstindex) {
        QString repo;
        for (int i=firstindex;i<=lastindex;i++) {
            if (rows[i].repo != "aur") {
                if (repo.isEmpty()) repo = rows[i].repo;
                rows[i].name = "";
            }
        }
        for (int i=firstindex;i<=lastindex;i++) {
            if (rows[i].repo == "aur") {
                rows[i].repo = repo;
                break;
            }
        }
    }
}

void PacmanItemModel::fix_sort_portion2(int firstindex,int lastindex) {
    if (rows[firstindex].version == rows[lastindex].version) return;

    QString version = rows[lastindex].version;

    bool isInstalled = false;
    for (int i=lastindex;i>=firstindex;i--) {
        if ((rows[i].getVersion() != version) && rows[i].isInstalled()) {
            isInstalled = true;
            break;
        }
    }

    if (!isInstalled) return;

    rows[lastindex].m_isUpdate = true;
}

void PacmanItemModel::sort() {
    if (rowCount() <= 0) return;

    qSort(rows.begin(),rows.end(),PacmanItemModel::pacman_model_asc_sort);

    int firstindex = 0;
    int lastindex = 0;
    int latestindex = rows.count()-1;
    if (latestindex == 0) lastindex = -1;
    while (lastindex <= latestindex) {
        lastindex = findLastIndex(firstindex,PacmanItemModel::pacman_model_asc_compare);
        if (lastindex > latestindex) lastindex = latestindex;
        fix_sort_portion(firstindex,lastindex);
        if (lastindex == latestindex) break;
        firstindex = lastindex+1;
    }

    for (int i=(rows.count()-1);i>=0;i--) {
        if (rows[i].name.isEmpty()) rows.removeAt(i);
    }

    firstindex = 0;
    lastindex = 0;
    latestindex = rows.count()-1;
    if (latestindex == 0) lastindex = -1;
    while (lastindex <= latestindex) {
        lastindex = findLastIndex(firstindex,PacmanItemModel::pacman_model_no_version_asc_compare);
        if (lastindex > latestindex) lastindex = latestindex;
        fix_sort_portion2(firstindex,lastindex);
        if (lastindex == latestindex) break;
        firstindex = lastindex+1;
    }

    provides_rows.clear();
    for (int i=(rows.count()-1);i>=0;i--) {
        QStringList provides = rows[i].listProvides();
        if (provides.count() > 0) {
            for (int j=0;j<provides.count();j++) {
                QString provider;
                QString version;
                PacmanEntry::parseNameVersion(provides[j],provider,version);
                provides_rows[provider.toLower()].insert(i);
            }
        }
    }
}


QList<int> PacmanItemModel::filterRecords(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo) {
    QList<int> list;
    for (int i=0;i<rows.count();i++) {
        if (group.isEmpty()) {
            if (((fItemId == FilterToolButton::IS_INSTALLED) && !rows[i].isInstalled()) || !rows[i].containsText(text,cItemId) || ((rows[i].repo != repo) && !repo.isEmpty())) list.append(i);
            else if (((fItemId == FilterToolButton::IS_NONINSTALLED) && rows[i].isInstalled()) || !rows[i].containsText(text,cItemId) || ((rows[i].repo != repo) && !repo.isEmpty())) list.append(i);
            else if (((fItemId == FilterToolButton::IS_NEEDUPDATE) && !rows[i].isUpdate()) || !rows[i].containsText(text,cItemId) || ((rows[i].repo != repo) && !repo.isEmpty())) list.append(i);
            else if (((fItemId == FilterToolButton::IS_ORPHANED) && !rows[i].isOrphaned()) || !rows[i].containsText(text,cItemId) || ((rows[i].repo != repo) && !repo.isEmpty())) list.append(i);
            else if (((fItemId == FilterToolButton::IS_MARKED) && !rows[i].isChosen()) || !rows[i].containsText(text,cItemId) || ((rows[i].repo != repo) && !repo.isEmpty())) list.append(i);
        }
        else{
            if (!rows[i].ownedByGroup(group) || !rows[i].containsText(text,cItemId) || ((rows[i].repo != repo) && !repo.isEmpty())) list.append(i);
        }
    }

    return list;
}

PacmanEntry & PacmanItemModel::row(const QModelIndex & index) {
    return rows[index.row()];
}

PacmanEntry PacmanItemModel::row(const QModelIndex & index) const {
    int idx = index.row();
    if ((idx >= 0) && (idx < rows.count())) return rows[idx];
    return PacmanEntry();
}

QModelIndex PacmanItemModel::firstFoundIndexByPackageName(const QString & package) const {
    QList<PacmanEntry>::const_iterator i = qBinaryFind(rows.begin(),rows.end(),PacmanEntry(package),pacman_model_no_version_asc_sort);
    if (i == rows.end()) {
        if (provides_rows.contains(package)) return index(*provides_rows[package].begin(),0);
        return QModelIndex();
    }

    return index(i-rows.begin(),0);
}

QModelIndex PacmanItemModel::installedProviderIndex(const QString & provider) const {
    if (provides_rows.contains(provider)) {
        QSetIterator<int> i(provides_rows[provider]);
        while (i.hasNext()) {
            int index = i.next();
            if (rows[index].isInstalled()) return this->index(index,0);
        }
    }
    return QModelIndex();
}

QModelIndex PacmanItemModel::installedIndexByPackageName(const QString & package) const {
    QList<PacmanEntry>::const_iterator i = qBinaryFind(rows.begin(),rows.end(),PacmanEntry(package),pacman_model_no_version_asc_sort);
    if (i == rows.end()) return QModelIndex();

    int firstindex = findFirstIndex(i-rows.begin(),pacman_model_no_version_asc_compare);
    int lastindex = findLastIndex(i-rows.begin(),pacman_model_no_version_asc_compare);

    for (int i=firstindex;i<=lastindex;i++) {
        QModelIndex ind = index(i,0);
        if (row(ind).isInstalled()) return ind;
    }

    return QModelIndex();
}

QModelIndex PacmanItemModel::indexByPackageNameVersion(const QString & name,const QString & ver) const {
    QList<PacmanEntry>::const_iterator i = qBinaryFind(rows.begin(),rows.end(),PacmanEntry(name,ver),pacman_model_asc_sort);
    if (i == rows.end()) return QModelIndex();

    return index(i-rows.begin(),0);
}

QModelIndex PacmanItemModel::indexByEntry(const PacmanEntry & entry) const {
    return indexByPackageNameVersion(entry.getName(),entry.getVersion());
}

QList<PacmanEntry::InstalledStatus> PacmanItemModel::findPackagesInstalledStatus(const QStringList & packages) const {
    QList<PacmanEntry::InstalledStatus> statuses;
    QString name;
    QString version;

    for (int i=0;i<packages.count();i++) {
        PacmanEntry::parseNameVersion(packages[i],name,version);
        QModelIndex index = installedIndexByPackageName(name);
        if (!index.isValid()) statuses.append(PacmanEntry::NOT_INSTALLED);
        else statuses.append(row(index).getStatus());
    }

    return statuses;
}

void PacmanItemModel::setFiles(const QString & package,const QStringList & files) {
    QModelIndex index = installedIndexByPackageName(package);
    if (!index.isValid()) return;

    row(index).setFiles(files);
}

QList<QModelIndex> PacmanItemModel::indexesCanReplaceInstalled() const {
    QString name;
    QString ver;
    QList<QModelIndex> ret;
    for (int i=(rows.count()-1);i>=0;i--) {
        if (rows[i].isInstalled() && !rows[i].isUpdate()) continue;
        if (rows[i].replaces.isEmpty()) continue;
        if ((ret.count() > 0) && (rows[i].name == rows[ret[ret.count()-1].row()].name)) continue;

        for (int j=0;j<rows[i].replaces.count();j++) {
            PacmanEntry::parseNameVersion(rows[i].replaces[j],name,ver);
            if (!ver.isEmpty()) continue;

            QModelIndex index = installedIndexByPackageName(name);
            if (index.isValid() && (rows[index.row()].name != rows[i].name)) {
                ret.append(this->index(i,0));
                break;
            }
        }
    }

    return ret;
}

bool PacmanItemModel::removeRows(int row,int count,const QModelIndex & /*parent*/) {
    for (int i=(row+count-1);i>=row;i--) {
        rows.removeAt(i);
    }
    return true;
}

QList<QModelIndex> PacmanItemModel::updatesList() const {
    QList<QModelIndex> indexes;
    for (int i=(rows.count()-1);i>=0;i--) {
        if (rows[i].isUpdate()) indexes.append(index(i,0));
    }

    return indexes;
}
