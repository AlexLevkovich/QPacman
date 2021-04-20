#include "packageview.h"
#include "themeicons.h"
#include "installbuttondelegate.h"
#include "static.h"
#include <QEvent>
#include <QFontMetrics>
#include <QHeaderView>

bool PackageItemModel::first_time_init = true;
QString PackageItemModel::DO_INSTALL_STR;
QString PackageItemModel::DO_INSTALL_ASDEPS_STR;
QString PackageItemModel::DO_INSTALL_ASDEPS_FORCE_STR;
QString PackageItemModel::DO_INSTALL_FORCE_STR;
QString PackageItemModel::DO_REINSTALL_STR;
QString PackageItemModel::DO_REINSTALL_ASDEPS_STR;
QString PackageItemModel::DO_UNINSTALL_STR;
QString PackageItemModel::DO_UNINSTALL_ALL_STR;
QIcon PackageItemModel::install_icon;
QIcon PackageItemModel::reinstall_icon;
QIcon PackageItemModel::uninstall_icon;
QIcon PackageItemModel::installedIcon;
QIcon PackageItemModel::notinstalledIcon;
QIcon PackageItemModel::updatedIcon;

PackageItemModel::PackageItemModel(PackageView * parent) : QAbstractItemModel(parent) {
    if (first_time_init) {
        first_time_init = false;
        DO_INSTALL_STR = tr("Install");
        DO_INSTALL_ASDEPS_STR = tr("Install as a dependency");
        DO_INSTALL_ASDEPS_FORCE_STR = tr("Install forcibly as a dependency");
        DO_INSTALL_FORCE_STR = tr("Install forcibly");
        DO_REINSTALL_STR = tr("Reinstall");
        DO_REINSTALL_ASDEPS_STR = tr("Reinstall as a dependency");
        DO_UNINSTALL_STR = tr("Uninstall");
        DO_UNINSTALL_ALL_STR = tr("Uninstall with dependencies");
        install_icon = ThemeIcons::get(ThemeIcons::PKG_INSTALL_MARK);
        reinstall_icon = ThemeIcons::get(ThemeIcons::PKG_REINSTALL_MARK);
        uninstall_icon = ThemeIcons::get(ThemeIcons::PKG_REMOVE_MARK);
        installedIcon = ThemeIcons::get(ThemeIcons::PKG_INSTALLED_MARK);
        notinstalledIcon = ThemeIcons::get(ThemeIcons::PKG_NONINSTALLED_MARK);
        updatedIcon = ThemeIcons::get(ThemeIcons::UPDATE_ITEM);
    }
    m_parent = parent;
}

QVariant PackageItemModel::headerData(int section,Qt::Orientation orientation,int role) const {
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

AlpmPackage PackageItemModel::installedPackageByName(const QString & name) const {
    AlpmPackage pkg(name);
    QModelIndex index = indexByPackageName(pkg);
    if (!index.isValid()) return AlpmPackage();

    for (int i=index.row();i<rows.count();i++) {
        if (!rows[i].isInstalled()) continue;
        return rows[i];
    }

    return AlpmPackage();
}

bool PackageItemModel::setHeaderData(int /*section*/,Qt::Orientation /*orientation*/,const QVariant & /*value*/,int /*role*/) {
    return false;
}

int PackageItemModel::columnCount(const QModelIndex & /*parent*/) const {
    return 5;
}

const QString PackageItemModel::status_text(AlpmPackage::UserChangeStatus status) {
    switch (status) {
    case AlpmPackage::DO_INSTALL:
    case AlpmPackage::DO_INSTALL_ASDEPS:
    case AlpmPackage::DO_INSTALL_ASDEPS_FORCE:
    case AlpmPackage::DO_INSTALL_FORCE:
        return DO_INSTALL_STR;
    case AlpmPackage::DO_REINSTALL:
    case AlpmPackage::DO_REINSTALL_ASDEPS:
        return DO_REINSTALL_STR;
    case AlpmPackage::DO_UNINSTALL:
    case AlpmPackage::DO_UNINSTALL_ALL:
        return DO_UNINSTALL_STR;
    default:
        return QString();
    }
}

const QString PackageItemModel::statusText(const AlpmPackage & pkg) {
    QString ret = status_text(pkg.changeStatus());
    if (ret.isEmpty()) ret = status_text(pkg.defaultStatus());
    return ret;
}

const QString PackageItemModel::statusTextHint(AlpmPackage::UserChangeStatus status) {
    switch (status) {
    case AlpmPackage::DO_INSTALL:
        return DO_INSTALL_STR;
    case AlpmPackage::DO_INSTALL_ASDEPS:
        return DO_INSTALL_ASDEPS_STR;
    case AlpmPackage::DO_INSTALL_ASDEPS_FORCE:
        return DO_INSTALL_ASDEPS_FORCE_STR;
    case AlpmPackage::DO_INSTALL_FORCE:
        return DO_INSTALL_FORCE_STR;
    case AlpmPackage::DO_REINSTALL:
        return DO_REINSTALL_STR;
    case AlpmPackage::DO_REINSTALL_ASDEPS:
        return DO_REINSTALL_ASDEPS_STR;
    case AlpmPackage::DO_UNINSTALL:
        return DO_UNINSTALL_STR;
    case AlpmPackage::DO_UNINSTALL_ALL:
        return DO_UNINSTALL_ALL_STR;
    default:
        return QString();
    }
}

const QIcon PackageItemModel::statusIcon(const AlpmPackage & pkg) {
    QIcon ret = status_icon(pkg.changeStatus());
    if (ret.isNull()) ret = status_icon(pkg.defaultStatus());
    return ret;
}

const QIcon PackageItemModel::status_icon(AlpmPackage::UserChangeStatus status) {
    switch (status) {
    case AlpmPackage::DO_INSTALL:
    case AlpmPackage::DO_INSTALL_ASDEPS:
    case AlpmPackage::DO_INSTALL_ASDEPS_FORCE:
    case AlpmPackage::DO_INSTALL_FORCE:
        return install_icon;
    case AlpmPackage::DO_REINSTALL:
    case AlpmPackage::DO_REINSTALL_ASDEPS:
        return reinstall_icon;
    case AlpmPackage::DO_UNINSTALL:
    case AlpmPackage::DO_UNINSTALL_ALL:
        return uninstall_icon;
    default:
        return QIcon();
    }
}

int PackageItemModel::rowCount(const QModelIndex & parent) const {
    return parent.isValid()?0:rows.count();
}

void PackageItemModel::appendRow(const AlpmPackage & pkg) {
    beginInsertRows(QModelIndex(),rows.count(),rows.count());
    rows.append(pkg);
    endInsertRows();
}

bool PackageItemModel::removeRows(int row,int count,const QModelIndex &parent) {
    if (count <= 0) return false;
    beginRemoveRows(parent,row,row+(count -1));
    if (row == 0 && count >= rows.count()) rows.clear();
    else {
        for (int i=row+(count -1);i>=row;i--) {
            rows.removeAt(i);
        }
    }
    endRemoveRows();
    return true;
}

bool PackageItemModel::setData(const QModelIndex & index,const QVariant & value,int role) {
    if ((role == Qt::DisplayRole) && (index.column() == Action)) {
        rows[index.row()].setChangeStatus((AlpmPackage::UserChangeStatus)value.toInt());
        return true;
    }

    return false;
}

QModelIndex PackageItemModel::index(int row, int column, const QModelIndex & /*parent*/) const {
    return createIndex(row,column);
}

QModelIndex PackageItemModel::parent(const QModelIndex & /*index*/) const {
    return QModelIndex();
}

void PackageItemModel::chooseRow(const QModelIndex & index,bool sel,AlpmPackage::UserChangeStatus status) {
    rows[index.row()].setChangeStatus(sel?((status == AlpmPackage::DO_NOTHING)?rows[index.row()].defaultStatus():status):AlpmPackage::DO_NOTHING);
}

AlpmPackage & PackageItemModel::row(const QModelIndex & index) {
    if (index.row() >= rows.count()) return null_pkg;
    return rows[index.row()];
}

bool PackageItemModel::pkg_name_version_less(const AlpmPackage & pkg1,const AlpmPackage & pkg2) {
    int ret;
    if ((ret = pkg1.name().compare(pkg2.name())) < 0) return true;
    if (ret > 0) return false;
    return (AlpmPackage::Dependence::pkg_vercmp(pkg1.version(),pkg2.version()) < 0);
}

bool PackageItemModel::pkg_name_less(const AlpmPackage & pkg1,const AlpmPackage & pkg2) {
    return (pkg1.name().compare(pkg2.name()) < 0);
}

QModelIndex PackageItemModel::indexByPackageNameVersion(const AlpmPackage & pkg) const {
    QList<AlpmPackage>::const_iterator it = binary_search_ex(rows.begin(),rows.end(),pkg,pkg_name_version_less);
    if (it == rows.end()) return QModelIndex();

    return index(it-rows.begin(),0);
}

QModelIndex PackageItemModel::indexByPackageName(const AlpmPackage & pkg) const {
    QList<AlpmPackage>::const_iterator it = binary_search_ex(rows.begin(),rows.end(),pkg,pkg_name_less);
    if (it == rows.end()) return QModelIndex();

    return index(it-rows.begin(),0);
}

QVariant PackageItemModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid()) return QVariant();
    if (index.row() >= rowCount()) return QVariant();

    switch (role) {
        case Qt::DisplayRole:
            if (index.column() == Name) return QVariant(rows[index.row()].name());
            else if (index.column() == Version) return QVariant(rows[index.row()].version());
            else if (index.column() == Repository) return QVariant(rows[index.row()].repo());
            else if (index.column() == Description) return QVariant(rows[index.row()].description());
            return ((PackageItemModel *)this)->statusText(rows[index.row()]);
        case Qt::DecorationRole:
            if (index.column() == Name) return (rows[index.row()].changeStatus() != AlpmPackage::DO_NOTHING)?((PackageItemModel *)this)->statusIcon(rows[index.row()]):(QVariant(rows[index.row()].isInstalled()?installedIcon:(rows[index.row()].isUpdate()?updatedIcon:notinstalledIcon)));
            if (index.column() == Action) return ((PackageItemModel *)this)->statusIcon(rows[index.row()]);
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
        case Qt::ToolTipRole:
            if (rows[index.row()].changeStatus() != AlpmPackage::DO_NOTHING) return ((PackageItemModel *)this)->statusTextHint(rows[index.row()].changeStatus());
        default:
            break;
    }

    return QVariant();
}

PackageView::PackageView(QWidget *parent) : QTreeView(parent) {
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setIconSize(quadroSize(fontMetrics().height()+4));
    setMouseTracking(true);

    setModel(m_model = new PackageItemModel(this));
    InstallButtonDelegate * delegate = new InstallButtonDelegate(this);
    setItemDelegate(delegate);

    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    for (int i=2;i<m_model->columnCount();i++) {
        header()->setSectionResizeMode(i,QHeaderView::ResizeToContents);
    }
    header()->setSectionResizeMode(1,QHeaderView::Stretch);
    header()->setSectionsMovable(false);

    history_disabled = false;
    history_index = -1;
    history_count = 0;

    connect(delegate,SIGNAL(rowChoosingStateChanged(const QModelIndex &)),this,SIGNAL(rowChoosingStateChanged(const QModelIndex &)));
    connect(&selectTimer,SIGNAL(timeout()),this,SLOT(selectTimeout()));
    connect(Alpm::instance(),&Alpm::package_queried,this,&PackageView::package_queried);
    connect(Alpm::instance(),SIGNAL(alpm_reopen()),this,SLOT(refreshRows()));
    connect(Alpm::instance(),SIGNAL(method_finished(const QString &,ThreadRun::RC)),this,SLOT(packages_queried(const QString &,ThreadRun::RC)));
    connect(Alpm::instance(),&Alpm::alpm_reopen,this,&PackageView::refresh_needed);
}

void PackageView::setModel(QAbstractItemModel * model) {
    QItemSelectionModel *sm = this->selectionModel();
    QAbstractItemModel *m = this->model();
    model->setParent(this);
    QTreeView::setModel(model);
    if (sm != NULL) sm->deleteLater();
    if (m != NULL) m->deleteLater();
}

void PackageView::markAll() {
    for (int i=0;i<m_model->rowCount();i++) {
        QModelIndex index = m_model->index(i,m_model->columnCount()-1);
        m_model->chooseRow(index,true);
        update(index);
        update(m_model->index(index.row(),0));
    }
    emit rowChoosingStateChanged(QModelIndex());
}

void PackageView::resetAll() {
    AlpmPackage::resetAllChangeStatuses();
    viewport()->repaint();
    emit rowChoosingStateChanged(QModelIndex());
}

void PackageView::clear() {
    m_model->removeRows(0,m_model->rows.count());
}

void PackageView::refreshRows(const QString & name,AlpmPackage::SearchFieldType fieldType,AlpmPackage::PackageFilter filter,const QString & group,const QString & repo) {
    if (!Alpm::instance()->queryPackages(name,fieldType,filter,group,repo)) return;
    clear();
    emit refreshBeginning();
    currentSelectionState = PackageView::SelectionState(name,fieldType,filter,group,repo);
    emit search_changed(name,fieldType,filter,group,repo);
}

void PackageView::refreshRows() {
    refreshRows(currentSelectionState);
}

void PackageView::package_queried(const AlpmPackage & pkg) {
    m_model->appendRow(pkg);
}

void PackageView::packages_queried(const QString &,ThreadRun::RC rc) {
    if (rc != ThreadRun::OK) return;
    if (currentSelectionState.package().isValid()) {
        selectPackageByState(currentSelectionState);
        currentSelectionState.setPackage(AlpmPackage());
    }
    emit refreshCompleted();
    history_disabled = false;
}

QModelIndex PackageView::selectedRow() const {
    QModelIndexList list = selectionModel()->selectedIndexes();
    if (list.count() <= 0) return QModelIndex();

    return list[0];
}

void PackageView::selectPackageByDep(const AlpmPackage::Dependence & dep) {
    AlpmPackage pkg;
    QModelIndex index = m_model->indexByPackageName(AlpmPackage(dep.name(),dep.version()));
    if (index.isValid()) {
        for (int i=index.row();i<m_model->rowCount();i++) {
            pkg = m_model->row(m_model->index(i,0));
            if (!dep.isAppropriate(pkg.name(),pkg.version())) return;
            selectionModel()->clearSelection();
            selectionModel()->select(index,QItemSelectionModel::Select|QItemSelectionModel::Rows);
            scrollTo(index);
            return;
        }
    }
    for (AlpmPackage & pkg: Alpm::instance()->findByPackageName(dep.name())) {
        if (dep.isAppropriate(pkg.name(),pkg.version())) {
            currentSelectionState = SelectionState();
            currentSelectionState.setPackage(pkg);
            refreshRows();
            return;
        }
    }
    for (AlpmPackage & pkg: Alpm::instance()->findByPackageNameProvides(dep)) {
        currentSelectionState = SelectionState();
        currentSelectionState.setPackage(pkg);
        refreshRows();
        return;
    }
}

void PackageView::selectPackageByState(const SelectionState & state) {
    if (!state.package().isValid()) return;
    QModelIndex index = m_model->indexByPackageNameVersion(state.package());
    if (index.isValid()) {
        selectionModel()->clearSelection();
        selectionModel()->select(index,QItemSelectionModel::Select|QItemSelectionModel::Rows);
        scrollTo(index);
    }
    else {
        currentSelectionState.setPackage(state.package());
        refreshRows(state);
    }
}

void PackageView::refreshRows(const PackageView::SelectionState & state) {
    refreshRows(state.m_name,state.m_fieldType,state.m_filter,state.m_group,state.m_repo);
}

void PackageView::selectTimeout() {
    selectTimer.stop();
    emit selectionChanged(m_model->row(selectTimer.property("index").toModelIndex()));
}

void PackageView::selectionChanged(const QItemSelection & selected,const QItemSelection & deselected) {
    QTreeView::selectionChanged(selected,deselected);
    if (!selected.isEmpty()) {
        QModelIndex index = selected.indexes()[0];
        if (!history_disabled) {
            history_count = history_index + 1;
            if (history_items.count() > history_count) {
                history_items[history_count] = currentSelectionState;
            }
            else history_items.append(currentSelectionState);
            history_index = history_count;
            history_items[history_index].setPackage(m_model->row(index));
        }
        selectTimer.stop();
        selectTimer.setProperty("index",index);
        selectTimer.start(200);
    }
}

void PackageView::refreshState(const SelectionState & state) {
    QModelIndex index = m_model->indexByPackageNameVersion(state.package());
    if (index.isValid()) {
        selectionModel()->clearSelection();
        selectionModel()->select(index,QItemSelectionModel::Select|QItemSelectionModel::Rows);
        scrollTo(index);
        return;
    }
    currentSelectionState = state;
    currentSelectionState.setPackage(state.package());
    refreshRows();
}

void PackageView::selectPrev() {
    if (!isSelectPrevPossible()) return;
    history_index--;
    history_disabled = true;
    currentSelectionState = history_items.at(history_index);
    currentSelectionState.setPackage(history_items.at(history_index).package());
    refreshState(currentSelectionState);
}

void PackageView::selectNext() {
    if (!isSelectNextPossible()) return;
    history_index++;
    history_disabled = true;
    currentSelectionState = history_items.at(history_index);
    currentSelectionState.setPackage(history_items.at(history_index).package());
    refreshState(currentSelectionState);
}

bool PackageView::isSelectPrevPossible() {
    return ((history_index-1) >= 0);
}

bool PackageView::isSelectNextPossible() {
    return ((history_index+1) < history_count);
}

void PackageView::markedPackages(QList<AlpmPackage> & install,QList<AlpmPackage> & install_asdeps,QList<AlpmPackage> & install_forced,QList<AlpmPackage> & removeall,QList<AlpmPackage> & remove) {
    install.clear();
    install_asdeps.clear();
    install_forced.clear();
    removeall.clear();
    remove.clear();
    if (m_model == NULL) return;
    for (AlpmPackage & row: Alpm::instance()->markedPackages()) {
        if (row.changeStatus() == AlpmPackage::DO_INSTALL ||
            row.changeStatus() == AlpmPackage::DO_REINSTALL ||
            row.changeStatus() == AlpmPackage::DO_INSTALL_FORCE) install.append(row);
        if (row.changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS ||
            row.changeStatus() == AlpmPackage::DO_REINSTALL_ASDEPS ||
            row.changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS_FORCE) install_asdeps.append(row);
        if (row.changeStatus() == AlpmPackage::DO_INSTALL_FORCE ||
            row.changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS_FORCE) install_forced.append(row);
        if (row.changeStatus() == AlpmPackage::DO_UNINSTALL_ALL) removeall.append(row);
        if (row.changeStatus() == AlpmPackage::DO_UNINSTALL) remove.append(row);
    }
}

PackageView::SelectionState::SelectionState(const PackageView::SelectionState & state) {
    *this = state;
}

PackageView::SelectionState::SelectionState(const QString & name,AlpmPackage::SearchFieldType fieldType,AlpmPackage::PackageFilter filter,const QString & group,const QString & repo) {
    m_name = name;
    m_fieldType = fieldType;
    m_filter = filter;
    m_group = group;
    m_repo = repo;
}

void PackageView::SelectionState::setPackage(const AlpmPackage & pkg) {
    m_pkg = pkg;
}

AlpmPackage PackageView::SelectionState::package() const {
    return m_pkg;
}

bool PackageView::SelectionState::operator==(const PackageView::SelectionState & state) const {
    return (m_name == state.m_name && m_fieldType == state.m_fieldType && m_filter == state.m_filter && m_group == state.m_group && m_repo == state.m_repo);
}

PackageView::SelectionState & PackageView::SelectionState::operator=(const PackageView::SelectionState & state) {
    m_name = state.m_name;
    m_fieldType = state.m_fieldType;
    m_filter = state.m_filter;
    m_group = state.m_group;
    m_repo = state.m_repo;
    return *this;
}

void PackageView::revertPackageReason(const QString & pkgname) {
    AlpmPackage pkg = m_model->installedPackageByName(pkgname);
    if (!pkg.isValid()) return;

    pkg.setReason((pkg.reason() == AlpmPackage::Explicit)?AlpmPackage::Depend:AlpmPackage::Explicit);
}

void PackageView::markPackageByNameToInstall(const QString & pkgname) {
    for (AlpmPackage & pkg: Alpm::instance()->findByPackageName(pkgname)) {
        if (pkg.isInstalled()) continue;
        pkg.setChangeStatus(pkg.defaultStatus());
        viewport()->repaint();
        return;
    }
}

void PackageView::refresh_needed() {
    clear();
    refreshRows();
}
