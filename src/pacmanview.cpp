/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanview.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QLayout>
#include "installbuttondelegate.h"
#include "packageretriver.h"
#include "dbrefresher.h"
#include "static.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QApplication>
#include <QDebug>

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

PacmanView::PacmanView(QWidget *parent) : QTreeView(parent) {
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setIconSize(Static::quadroSize(fontMetrics().height()+4));
    setMouseTracking(true);

    history_disabled = false;
    history_index = -1;
    scrollPressed = false;
    m_is_refreshing = false;
    m_model = new PacmanItemModel(this);
    setModel(m_model);
    delegate = new InstallButtonDelegate(this);
    installEventFilter(delegate);

    header()->setStretchLastSection(false);
    setHeaderSections();
    header()->setSectionsMovable(false);
    setItemDelegate(delegate);

    connect(delegate,SIGNAL(rowChoosingStateChanged(const QModelIndex &)),this,SIGNAL(rowChoosingStateChanged(const QModelIndex &)));
    connect((QObject *)verticalScrollBar(),SIGNAL(sliderReleased()),this,SLOT(onScrollBarReleased()));
    connect((QObject *)verticalScrollBar(),SIGNAL(sliderPressed()),this,SLOT(onScrollBarPressed()));
    connect((QObject *)verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(onScrollBarValueChanged(int)));
    connect(&scrollTimer,SIGNAL(timeout()),this,SLOT(scrollTimeout()));

    QMetaObject::invokeMethod(this,"refresh",Qt::QueuedConnection);
}

bool PacmanView::isRefreshing() const {
    return m_is_refreshing;
}

void PacmanView::read_packages_finished() {
    m_model->refreshRows();
    emit refreshCompleted();

    if (!_selEntry.name().isEmpty()) selectPackageByNameVersion(_selEntry.name(),_selEntry.version());
    if (m_recover_operations) recoverMarkedPackages();
    m_is_refreshing = false;
}

void PacmanView::refresh(bool recover_operations) {
    bool ok = true;
    emit refreshMaybeStarted(&ok);
    if (!ok) return;

    m_recover_operations = recover_operations;
    m_is_refreshing = true;

    QMap<AlpmPackage *,AlpmPackage::UserChangeStatus> pkgs;
    markedPackages(pkgs);
    saveMarkedPackages(pkgs);

    QItemSelectionModel * sel_model = selectionModel();
    if (sel_model != NULL) {
        QModelIndexList sel_items = sel_model->selectedRows();
        if (sel_items.count() > 0) _selEntry = AlpmPackage::Dependence(row(sel_items[0]));
    }

    if (m_model != NULL) m_model->removeRows(0,m_model->rowCount());
    reset();
    repaint();

    emit refreshBeginning();

    PackageRetriver * pacrepreader = new PackageRetriver(this);
    connect(pacrepreader,SIGNAL(listing_packages_completed()),this,SLOT(read_packages_finished()));
}

void PacmanView::applyFilter(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo) {
    QList<int> indexes = m_model->filterRecords(text,cItemId,fItemId,(fItemId == FilterToolButton::IS_GROUP)?group:"",(repo == Static::RepoAll_Str)?"":repo);
    showAllRows();
    for (int i=0;i<indexes.count();i++) {
        setRowHidden(indexes[i],QModelIndex(),true);
    }
}

void PacmanView::selectionChanged(const QItemSelection & selected,const QItemSelection & deselected) {
    QTreeView::selectionChanged(selected,deselected);
    if (!selected.isEmpty()) {
        QModelIndex index = selected.indexes()[0];
        if (!history_disabled) {
            if (history_index >= 0) history_items.resize(history_index+1);
            history_items.append(AlpmPackage::Dependence(m_model->row(index)));
            history_index = history_items.count() - 1;
        }
        emit selectionChanged(index);
    }
}

void PacmanView::selectPrev() {
    if (!isSelectPrevPossible()) return;
    history_index--;
    history_disabled = true;
    AlpmPackage::Dependence dep(history_items.at(history_index));
    selectPackageByNameVersion(dep.name(),dep.version());
    history_disabled = false;
}

void PacmanView::selectNext() {
    if (!isSelectNextPossible()) return;
    history_index++;
    history_disabled = true;
    AlpmPackage::Dependence dep(history_items.at(history_index));
    selectPackageByNameVersion(dep.name(),dep.version());
    history_disabled = false;
}

bool PacmanView::isSelectPrevPossible() {
    return ((history_index-1) >= 0);
}

bool PacmanView::isSelectNextPossible() {
    return ((history_index+1) < history_items.count());
}

void PacmanView::selectPackage(const AlpmPackage::Dependence & pkg) {
    QModelIndex index = m_model->firstFoundIndexByDep(pkg);
    if (index.isValid()) {
        selectionModel()->select(index,QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
        scrollTo(index);
    }
}

void PacmanView::markPackageByNameToInstall(const QString & package) {
    QModelIndex index = m_model->firstFoundUninstalledIndexByPackageName(package);
    if (index.isValid() && !m_model->row(index)->isChosen()) {
        markRow(index);
        emit rowChoosingStateChanged(index);
    }
}

void PacmanView::selectPackageByNameVersion(const QString & name,const QString & version) {
    QModelIndex index = m_model->indexByPackageNameVersion(name,version);
    if (index.isValid()) {
        selectionModel()->clearSelection();
        selectionModel()->select(index,QItemSelectionModel::Select|QItemSelectionModel::Rows);
        scrollTo(index);
    }
}

void PacmanView::showAllRows() {
    for (int i=0;i<m_model->rowCount();i++) {
        setRowHidden(i,QModelIndex(),false);
    }
}

void PacmanView::markAll() {
    for (int i=0;i<m_model->rowCount();i++) {
        if (!isRowHidden(i,QModelIndex())) {
            QModelIndex index = m_model->index(i,m_model->columnCount()-1);
            m_model->chooseRow(index,true);
            update(index);
            update(m_model->index(index.row(),0));
        }
    }
    emit rowChoosingStateChanged(QModelIndex());
}

void PacmanView::markRow(const QModelIndex & _index) {
    if (!_index.isValid()) return;

    QModelIndex index = m_model->index(_index.row(),m_model->columnCount()-1);
    m_model->chooseRow(index,true);
    update(index);
    update(m_model->index(index.row(),0));
}

void PacmanView::resetRow(const QModelIndex & _index) {
    if (!_index.isValid()) return;

    QModelIndex index = m_model->index(_index.row(),m_model->columnCount()-1);
    m_model->chooseRow(index,false);
    update(index);
    update(m_model->index(index.row(),0));
}

bool PacmanView::isVisibleIndex(const QModelIndex & index) const {
    return viewport()->rect().intersects(visualRect(index));
}

int PacmanView::visibleRowIndex(const QModelIndex & index) const {
    if (!isVisibleIndex(index)) return -1;

    QModelIndex next_index = indexAt(QPoint(0,0));
    if (next_index == index) return 0;

    for (int i=1;(next_index = indexBelow(next_index)).isValid() && isVisibleIndex(next_index);i++) {
        if (next_index == index) return i;
    }

    return -1;
}

void PacmanView::resetAll() {
    for (int i=0;i<m_model->rowCount();i++) {
        if (!isRowHidden(i,QModelIndex())) {
            QModelIndex index = m_model->index(i,m_model->columnCount()-1);
            m_model->chooseRow(index,false);
            update(index);
            update(m_model->index(index.row(),0));
        }
    }
    emit rowChoosingStateChanged(QModelIndex());
}

void PacmanView::markedPackages(QList<AlpmPackage *> & install,QList<AlpmPackage *> & install_asdeps,QList<AlpmPackage *> & install_forced,QList<AlpmPackage *> & removeall,QList<AlpmPackage *> & remove) {
    install.clear();
    install_asdeps.clear();
    install_forced.clear();
    removeall.clear();
    remove.clear();
    if (m_model == NULL) return;
    for (int i=0;i<m_model->rowCount();i++) {
        AlpmPackage * row = m_model->row(m_model->index(i,0));
        if (!row->isChosen()) continue;
        if (row->changeStatus() == AlpmPackage::DO_INSTALL ||
            row->changeStatus() == AlpmPackage::DO_REINSTALL ||
            row->changeStatus() == AlpmPackage::DO_INSTALL_FORCE) install.append(row);
        if (row->changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS ||
            row->changeStatus() == AlpmPackage::DO_REINSTALL_ASDEPS ||
            row->changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS_FORCE) install_asdeps.append(row);
        if (row->changeStatus() == AlpmPackage::DO_INSTALL_FORCE ||
            row->changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS_FORCE) install_forced.append(row);
        if (row->changeStatus() == AlpmPackage::DO_UNINSTALL_ALL) removeall.append(row);
        if (row->changeStatus() == AlpmPackage::DO_UNINSTALL) remove.append(row);
    }
}

void PacmanView::markedPackages(QMap<AlpmPackage *,AlpmPackage::UserChangeStatus> & pkgs) {
    pkgs.clear();
    if (m_model == NULL) return;
    for (int i=0;i<m_model->rowCount();i++) {
        AlpmPackage * row = m_model->row(m_model->index(i,0));
        if (!row->isChosen()) continue;
        pkgs[row] = row->changeStatus();
    }
}

void PacmanView::recoverMarkedPackages() {
    qint64 index;
    QModelIndex model_index;
    for (QMap<PkgParm,AlpmPackage::UserChangeStatus>::iterator i = m_pre_refresh_pkgs.begin();i != m_pre_refresh_pkgs.end();++i) {
        index = m_model->findCacheIndexByPackageNameVersionRepo(i.key().name,i.key().version,i.key().repo);
        if (index == -1) continue;
        model_index = m_model->index(index,0);
        if (m_model->row(model_index)->setChangeStatus(i.value())) markRow(model_index);
    }
}

void PacmanView::saveMarkedPackages(const QMap<AlpmPackage *,AlpmPackage::UserChangeStatus> & pkgs) {
    PkgParm parm;
    m_pre_refresh_pkgs.clear();
    for (QMap<AlpmPackage *,AlpmPackage::UserChangeStatus>::const_iterator i = pkgs.cbegin();i != pkgs.end();++i) {
        parm.name = i.key()->name();
        parm.version = i.key()->version();
        parm.repo = i.key()->repo();
        m_pre_refresh_pkgs[parm] = i.value();
    }
}

AlpmPackage * PacmanView::row(const QModelIndex & index) {
    return m_model->row(index);
}

QStringList PacmanView::files(const QString & package) {
    QModelIndex index = m_model->installedIndexByPackageName(package);
    if (!index.isValid()) return QStringList();

    return m_model->row(index)->files();
}

void PacmanView::revertReason(const QString & package) {
    QModelIndex index = m_model->installedIndexByPackageName(package);
    if (!index.isValid()) return;

    AlpmPackage * pkg = m_model->row(index);
    pkg->setReason((pkg->reason() == AlpmPackage::Explicit)?AlpmPackage::Depend:AlpmPackage::Explicit);
}

void PacmanView::setHeaderSections() {
    header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    for (int i=2;i<m_model->columnCount();i++) {
        header()->setSectionResizeMode(i,QHeaderView::ResizeToContents);
    }
    header()->setSectionResizeMode(1,QHeaderView::Stretch);    
}

void PacmanView::onScrollBarReleased() {
    scrollPressed = false;
    setHeaderSections();
    QMetaObject::invokeMethod(viewport(),"update",Qt::QueuedConnection);
}

void PacmanView::onScrollBarPressed() {
    scrollPressed = true;
}

void PacmanView::scrollTimeout() {
    scrollTimer.stop();
    if (!scrollPressed) setHeaderSections();
}

void PacmanView::onScrollBarValueChanged(int /*value*/) {
    scrollTimer.stop();
    scrollTimer.start(500);
    QMetaObject::invokeMethod(viewport(),"update",Qt::QueuedConnection);
}

QModelIndex PacmanView::indexByPackageNameVersion(const QString & name,const QString & ver) const {
    return m_model->indexByPackageNameVersion(name,ver);
}

QModelIndex PacmanView::installedIndexByPackageName(const QString & package) const {
    return m_model->installedIndexByPackageName(package);
}

QModelIndex PacmanView::selectedRow() const {
    QModelIndexList list = selectionModel()->selectedIndexes();
    if (list.count() <= 0) return QModelIndex();

    return list[0];
}
