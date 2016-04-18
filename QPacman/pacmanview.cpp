/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanview.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QLayout>
#include "pacmanrepositoryreader.h"
#include "pacmanfileslistreader.h"
#include "pacmanpackagereasonchanger.h"
#include "installbuttondelegate.h"
#include "static.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QGraphicsView>

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

PacmanView::PacmanView(QWidget *parent) : QTreeView(parent) {
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setIconSize(QSize(22,22));
    setMouseTracking(true);

    waitView = NULL;
    prev_model = NULL;
    prev_sel_model = NULL;
    model = NULL;
    scrollPressed = false;
    delegate = new InstallButtonDelegate(this);
    connect(delegate,SIGNAL(rowChoosingStateChanged(const QModelIndex &)),this,SIGNAL(rowChoosingStateChanged(const QModelIndex &)));
    connect((QObject *)verticalScrollBar(),SIGNAL(sliderReleased()),this,SLOT(onScrollBarReleased()));
    connect((QObject *)verticalScrollBar(),SIGNAL(sliderPressed()),this,SLOT(onScrollBarPressed()));
    connect((QObject *)verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(onScrollBarValueChanged(int)));
    connect(&scrollTimer,SIGNAL(timeout()),this,SLOT(scrollTimeout()));

    QMetaObject::invokeMethod(this,"refresh",Qt::QueuedConnection);
}

PacmanView::~PacmanView() {
    delete delegate;
    delete model;
}

void PacmanView::read_package(const PacmanEntry & item) {
    model->addRow(item);
}

void PacmanView::read_files_finished(PacmanProcessReader * ptr) {
    delete ptr;

    setModel(model);
    delegate->setModel(model);
    if (prev_model != NULL) delete prev_model;
    if (prev_sel_model != NULL) delete prev_sel_model;

    header()->setStretchLastSection(false);
    setHeaderSections();
#if QT_VERSION >= 0x050000
    header()->setSectionsMovable(false);
#else
    header()->setMovable(false);
#endif    

    if (waitView != NULL) {
        waitView->setVisible(false);
        setVisible(true);
    }


    emit canFillRepos(model->getRepos());
    emit canFillFilters(model->getGroups());
    emit refreshCompleted();

    if (_selEntry.isValid()) selectPackageByEntry(_selEntry);

    emit enableActions(true);
}

void PacmanView::checkReplaces() {
    QList<QModelIndex> indexes = model->indexesCanReplaceInstalled();
    if (indexes.count() > 0) {
        for (int i=0;i<indexes.count();i++) {
            PacmanEntry package = model->row(indexes[i]);
            if (QMessageBox::warning(this,"Question...",tr("%1 replaces %2.\nDo you want to mark it for installation?").arg(package.getName()+"-"+package.getVersion()).arg(package.listReplaces().join(" and ")),QMessageBox::Yes,QMessageBox::No) == QMessageBox::No) continue;
            model->chooseRow(indexes[i],true);
            update(indexes[i]);
            emit rowChoosingStateChanged(indexes[i]);
        }
    }
}

void PacmanView::files_ready(const QString & package,const QStringList & files) {
    model->setFiles(package,files);
}

void PacmanView::read_packages_finished(PacmanProcessReader * ptr) {
    model->sort();
    delete ptr;

    PacmanFilesListReader * filesreader = new PacmanFilesListReader();
    connect(filesreader,SIGNAL(files_ready(const QString &,const QStringList &)),this,SLOT(files_ready(const QString &,const QStringList &)));
    connect(filesreader,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(read_files_finished(PacmanProcessReader *)));
}

void PacmanView::refresh() {
    emit enableActions(false);
    if (waitView != NULL) {
        waitView->setVisible(true);
        setVisible(false);
    }

    prev_sel_model = selectionModel();
    _selEntry = PacmanEntry();
    if (prev_sel_model != NULL) {
        QModelIndexList sel_items = prev_sel_model->selectedRows();
        if (sel_items.count() > 0) _selEntry = row(sel_items[0]);
    }

    prev_model = model;
    if (model != NULL) model->removeRows(0,model->rowCount());
    reset();

    model = new PacmanItemModel(this);

    for (int i=0;i<model->columnCount();i++) {
        setItemDelegateForColumn(i,delegate);
    }

    PacmanRepositoryReader * pacrepreader = new PacmanRepositoryReader();
    connect(pacrepreader,SIGNAL(read_package(const PacmanEntry &)),this,SLOT(read_package(const PacmanEntry &)));
    connect(pacrepreader,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(read_packages_finished(PacmanProcessReader *)));
}

void PacmanView::onSearchChanged(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo) {
    QList<int> indexes = model->filterRecords(text,cItemId,fItemId,(fItemId == FilterToolButton::IS_GROUP)?group:"",(repo == Static::RepoAll_Str)?"":repo);
    showAllRows();
    for (int i=0;i<indexes.count();i++) {
        setRowHidden(indexes[i],QModelIndex(),true);
    }
}

void PacmanView::selectionChanged(const QItemSelection & selected,const QItemSelection & deselected) {
    QTreeView::selectionChanged(selected,deselected);
    if (!selected.isEmpty()) emit selectionChanged(selected.indexes()[0]);
}

void PacmanView::selectPackageByName(const QString & package) {
    QModelIndex index = model->firstFoundIndexByPackageName(package);
    if (index.isValid()) {
        selectionModel()->clearSelection();
        selectionModel()->select(index,QItemSelectionModel::Select|QItemSelectionModel::Rows);
        scrollTo(index);
    }
}

void PacmanView::selectPackageByEntry(const PacmanEntry & entry) {
    QModelIndex index = model->indexByEntry(entry);
    if (index.isValid()) {
        selectionModel()->clearSelection();
        selectionModel()->select(index,QItemSelectionModel::Select|QItemSelectionModel::Rows);
        scrollTo(index);
    }
}

void PacmanView::showAllRows() {
    for (int i=0;i<model->rowCount();i++) {
        setRowHidden(i,QModelIndex(),false);
    }
}

void PacmanView::markAll() {
    for (int i=0;i<model->rowCount();i++) {
        if (!isRowHidden(i,QModelIndex())) {
            QModelIndex index = model->index(i,model->columnCount()-1);
            model->chooseRow(index,true);
            update(index);
            update(model->index(index.row(),0));
        }
    }
    emit rowChoosingStateChanged(QModelIndex());
}

void PacmanView::markRow(const QModelIndex & _index) {
    if (!_index.isValid()) return;

    QModelIndex index = model->index(_index.row(),model->columnCount()-1);
    model->chooseRow(index,true);
    update(index);
    update(model->index(index.row(),0));
}

void PacmanView::resetRow(const QModelIndex & _index) {
    if (!_index.isValid()) return;

    QModelIndex index = model->index(_index.row(),model->columnCount()-1);
    model->chooseRow(index,false);
    update(index);
    update(model->index(index.row(),0));
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
    for (int i=0;i<model->rowCount();i++) {
        if (!isRowHidden(i,QModelIndex())) {
            QModelIndex index = model->index(i,model->columnCount()-1);
            model->chooseRow(index,false);
            update(index);
            update(model->index(index.row(),0));
        }
    }
    emit rowChoosingStateChanged(QModelIndex());
}

QList<PacmanEntry> PacmanView::markedPackagesToInstall() const {
    QList<PacmanEntry> list;
    for (int i=0;i<model->rowCount();i++) {
        PacmanEntry row = model->row(model->index(i,0));
        if (row.isChosen()) {
            if (row.getChangeStatus() != PacmanEntry::DO_UNINSTALL_ALL &&
                row.getChangeStatus() != PacmanEntry::DO_UNINSTALL) list.append(row);
        }
    }

    return list;
}

QList<PacmanEntry> PacmanView::markedPackagesToRemoveAll() const {
    QList<PacmanEntry> list;
    for (int i=0;i<model->rowCount();i++) {
        PacmanEntry row = model->row(model->index(i,0));
        if (row.isChosen()) {
            if (row.getChangeStatus() == PacmanEntry::DO_UNINSTALL_ALL) list.append(row);
        }
    }

    return list;
}

QList<PacmanEntry> PacmanView::markedPackagesToRemove() const {
    QList<PacmanEntry> list;
    for (int i=0;i<model->rowCount();i++) {
        PacmanEntry row = model->row(model->index(i,0));
        if (row.isChosen()) {
            if (row.getChangeStatus() == PacmanEntry::DO_UNINSTALL) list.append(row);
        }
    }

    return list;
}

PacmanEntry PacmanView::row(const QModelIndex & index) const {
    return model->row(index);
}

QList<PacmanEntry::InstalledStatus> PacmanView::findPackagesInstalledStatus(const QStringList & packages) const {
    return model->findPackagesInstalledStatus(packages);
}

QStringList PacmanView::files(const QString & package) {
    QModelIndex index = model->installedIndexByPackageName(package);
    if (!index.isValid()) return QStringList();

    return model->row(index).listFiles();
}

void PacmanView::revertReason(const QString & package) {
    QModelIndex index = model->installedIndexByPackageName(package);
    if (!index.isValid()) return;

    PacmanPackageReasonChanger reasonchanger(package,model->row(index).isExplicitly());
    reasonchanger.waitToComplete();
    if (reasonchanger.exitCode() == 0) model->row(index).revertStatus();
}

void PacmanView::setHeaderSections() {
#if QT_VERSION >= 0x050000
    header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    for (int i=2;i<model->columnCount();i++) {
        header()->setSectionResizeMode(i,QHeaderView::ResizeToContents);
    }
    header()->setSectionResizeMode(1,QHeaderView::Stretch);    
#else    
    header()->setResizeMode(0,QHeaderView::ResizeToContents);
    for (int i=2;i<model->columnCount();i++) {
        header()->setResizeMode(i,QHeaderView::ResizeToContents);
    }
    header()->setResizeMode(1,QHeaderView::Stretch);
#endif    
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

QStringList PacmanView::updateNamesList() const {
    QList<QModelIndex> indexes = model->updatesList();
    QStringList names;
    for (int i=0;i<indexes.count();i++) {
        PacmanEntry entry = row(indexes[i]);
        names.append(entry.getName());
    }

    return names;
}

QModelIndex PacmanView::indexByPackageNameVersion(const QString & name,const QString & ver) const {
    return model->indexByPackageNameVersion(name,ver);
}

QModelIndex PacmanView::installedIndexByPackageName(const QString & package) const {
    return model->installedIndexByPackageName(package);
}

QModelIndex PacmanView::selectedRow() const {
    QModelIndexList list = selectionModel()->selectedIndexes();
    if (list.count() <= 0) return QModelIndex();

    return list[0];
}

void PacmanView::keyPressEvent(QKeyEvent * event) {
    QTreeView::keyPressEvent(event);

    QModelIndex index = selectedRow();
    if (!index.isValid()) return;
    index = model->index(index.row(),model->columnCount()-1);
    QStyleOptionViewItem option;
    option.rect = visualRect(index);

    switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            QMouseEvent m_event(QEvent::MouseButtonRelease,option.rect.topLeft(),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
            delegate->editorEvent(&m_event,model,option,index);
            break;
        }
        case Qt::Key_Right:
            delegate->editorEvent(event,model,option,index);
            break;
        default:
            break;
    }
}

