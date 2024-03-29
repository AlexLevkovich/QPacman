/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "widgetgroup.h"
#include "exclusiveactiongroup.h"
#include "themeicons.h"
#include "searchwidget.h"
#include "messagedialog.h"
#include "dbrefresher.h"
#include "libalpm.h"
#include "optionaldepsdlg.h"
#include "packageinstaller.h"
#include "actionapplier.h"
#include "packagedownloader.h"
#include <QMessageBox>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent) , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    wasInit = false;
    optdep = nullptr;

    view_group = new WidgetGroup(this);
    view_group->add(ui->splitter);
    view_group->add(ui->progressView);
    view_group->add(ui->waitView);
    view_group->add(ui->logWindow);
    view_group->add(ui->infoBrowser);
    view_group->add(ui->settingsBrowser);

    acgroup = new ExclusiveActionGroup(this);
    acgroup->addAction(ui->actionLog);
    acgroup->addAction(ui->actionInfo);
    acgroup->addAction(ui->actionSettings);

    ui->actionApply->setIcon(ThemeIcons::get(ThemeIcons::INSTALL));
    ui->actionReset->setIcon(ThemeIcons::get(ThemeIcons::UNDO_OR_RESET));
    ui->actionMark_All->setIcon(ThemeIcons::get(ThemeIcons::PKG_SELECT_ALL));
    ui->actionRefresh->setIcon(ThemeIcons::get(ThemeIcons::UPDATE_REPOS));
    ui->actionLog->setIcon(ThemeIcons::get(ThemeIcons::LOG_VIEW));
    ui->actionInfo->setIcon(ThemeIcons::get(ThemeIcons::IMPORTANT));
    ui->actionSettings->setIcon(ThemeIcons::get(ThemeIcons::CONFIGURE));
    ui->actionRefreshList->setIcon(ThemeIcons::get(ThemeIcons::REFRESH));
    ui->actionCacheCleanUp->setIcon(ThemeIcons::get(ThemeIcons::CLEAN_CACHE));
    ui->actionPrevious->setIcon(ThemeIcons::get(ThemeIcons::PREV));
    ui->actionNext->setIcon(ThemeIcons::get(ThemeIcons::NEXT));
    ui->actionFullUpdate->setIcon(ThemeIcons::get(ThemeIcons::SYNC));
    ui->actionCancel->setIcon(ThemeIcons::get(ThemeIcons::CANCEL));

    ui->actionApply->setVisible(true);
    ui->actionMark_All->setVisible(true);
    ui->actionsToolBar->insertSeparator(ui->actionLog);
    ui->actionReset->setVisible(true);
    ui->actionFullUpdate->setVisible(true);
    ui->actionRefresh->setVisible(true);

    ui->waitView->setVisible(false);
    ui->progressView->setVisible(false);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addWidget((searchWidget = new SearchWidget(ui->mainToolBar)));
    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->actionsToolBar->insertWidget(ui->actionCancel,empty);
    ui->actionLog->setVisible(true);
    ui->actionApply->setEnabled(false);
    ui->actionReset->setEnabled(false);

    initCombos();

    connect(ui->settingsBrowser,&SettingsTextBrowser::dbsRefreshNeeded,this,&MainWindow::on_actionRefresh_triggered);
    connect(Alpm::instance(),&Alpm::do_start_dbrefresher,this,&MainWindow::onTrayRefresherStart);
    connect(Alpm::instance(),&Alpm::do_start_package_updater,this,&MainWindow::onTrayUpdaterStart);
    connect(ui->actionApply,&QAction::triggered,this,&MainWindow::onActionApply);
    connect(ui->filesTree,&FilesListWidget::downloadRequested,this,&MainWindow::onDownloadRequested);
    connect(ui->packetView,&PackageView::refreshBeginning,this,&MainWindow::onRefreshBeginning);
    connect(ui->packetView,&PackageView::refreshCompleted,this,&MainWindow::onRefreshCompleted);
    connect(searchWidget,&SearchWidget::search_changed,this,&MainWindow::onSearchChanged);
    connect(ui->packetView,&PackageView::search_changed,this,&MainWindow::onViewSearchChanged);
    connect(ui->pacInfoView,&PacmanInfoBrowser::packageUrlSelected,this,&MainWindow::onPackageUrlSelected);
    connect(ui->pacInfoView,&PacmanInfoBrowser::reasonUrlSelected,this,&MainWindow::onReasonUrlSelected);
    connect(ui->pacInfoView,&PacmanInfoBrowser::groupUrlSelected,this,&MainWindow::onGroupUrlSelected);
    connect(ui->packetView,SIGNAL(selectionChanged(AlpmPackage)),this,SLOT(onViewSelectionChanged(AlpmPackage)));
    connect(ui->packetView,&PackageView::rowChoosingStateChanged,this,&MainWindow::onRowChoosingStateChanged);
}

MainWindow::~MainWindow() {
    delete ui;
}

static const QStringList reposToStringList(const QList<Alpm::Repo> & repos) {
    QStringList ret;
    for (const Alpm::Repo & repo: repos) ret.append(repo.name());
    return ret;
}

void MainWindow::initCombos() {
    SearchWidget * sw = ui->mainToolBar->findSearchWidget();
    sw->fillFiltersInCombo(Alpm::instance()->groups());
    sw->fillReposInCombo(reposToStringList(Alpm::instance()->repos()));
}

void MainWindow::onRefreshBeginning() {
    enableActions(false);
    view_group->setCurrent(ui->waitView);
}

void MainWindow::onRefreshCompleted() {
    bool action_state = actionApplyState();
    ui->actionApply->setEnabled(action_state);
    ui->actionReset->setEnabled(action_state);
    ui->filesTree->clear();
    ui->pacInfoView->clear();
    enableActions(true);
    acgroup->uncheckAllActions();
    view_group->setCurrent(ui->splitter);
}

static const QList<QAction *> childrenActions(QObject * main_object) {
    QList<QAction *> actions;
    QObjectList listObjects = main_object->children();
    for (int i=0;i<listObjects.count();i++) {
        actions.append(childrenActions(listObjects[i]));
        if (listObjects[i]->inherits("QAction")) actions.append((QAction *)listObjects[i]);
    }

    return actions;
}

void MainWindow::enableActions(bool flag) {
    QList<QAction *> actions = childrenActions(this);
    for (int i=0;i<actions.count();i++) {
        actions[i]->setEnabled(flag);
    }
    bool action_state = actionApplyState();
    ui->actionApply->setEnabled(!flag?false:action_state);
    ui->actionReset->setEnabled(!flag?false:action_state);
    ui->actionNext->setEnabled(!flag?false:ui->packetView->isSelectNextPossible());
    ui->actionPrevious->setEnabled(!flag?false:ui->packetView->isSelectPrevPossible());
}

bool MainWindow::actionApplyState() {
    return Alpm::instance()->areMarkedPackages();
}

void MainWindow::onViewSelectionChanged(const AlpmPackage & pkg) {
    if (!pkg.isValid()) return;
    ui->pacInfoView->fillByInfo(pkg);
    ui->filesTree->fill(pkg);
    ui->actionNext->setEnabled(ui->packetView->isSelectNextPossible());
    ui->actionPrevious->setEnabled(ui->packetView->isSelectPrevPossible());
    ui->actionNext->setEnabled(ui->packetView->isSelectNextPossible());
    ui->actionPrevious->setEnabled(ui->packetView->isSelectPrevPossible());
}

void MainWindow::onReasonUrlSelected(const QString & pkgname) {
    ui->packetView->revertPackageReason(pkgname);
    ui->pacInfoView->refill();
    if (ui->mainToolBar->findSearchWidget()->filter() == FilterToolButton::IS_ORPHANED) ui->packetView->refreshRows();
}

void MainWindow::onGroupUrlSelected(const QString & group) {
    searchWidget->setFilter(FilterToolButton::IS_GROUP,group);
    searchWidget->clearSearchText();
}

void MainWindow::onViewSearchChanged(const QString & text,AlpmPackage::SearchFieldType cItemId,AlpmPackage::PackageFilter fItemId,const QString & group,const QString & repo) {
    searchWidget->setSearchText(text);
    searchWidget->setCategory((CategoryToolButton::ItemId)cItemId);
    searchWidget->setFilter((FilterToolButton::ItemId)fItemId,group);
    searchWidget->setRepo(repo);
}

void MainWindow::onSearchChanged(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo) {
    ui->packetView->refreshRows(text,(AlpmPackage::SearchFieldType)cItemId,(AlpmPackage::PackageFilter)fItemId,group,repo);
}

void MainWindow::onPackageUrlSelected(const QString & name,const QString & version,int operation) {
    ui->packetView->selectPackageByDep(AlpmPackage::Dependence(name,(AlpmPackage::CompareOper)operation,version));
}

void MainWindow::on_actionPrevious_triggered() {
    ui->packetView->selectPrev();
}

void MainWindow::on_actionNext_triggered() {
    ui->packetView->selectNext();
}

void MainWindow::on_actionRefreshList_triggered(bool) {
    ui->packetView->refreshRows();
}

void MainWindow::on_actionCacheCleanUp_triggered() {
    if (QMessageBox::warning(this,tr("Warning..."),tr("The contents of cache directory will be removed.\nAre you sure to continue?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::No) return;
    if (Alpm::instance()->cleanCacheDirs()) MessageDialog::post(tr("The contents of cache directory was removed succesfully!"),QString(),tr("Information..."));
    else ErrorDialog::post(tr("The contents of cache directory was not removed!"),Alpm::instance()->lastError(),tr("Error..."));
}

void MainWindow::on_actionMark_All_triggered() {
    ui->packetView->markAll();
}

void MainWindow::on_actionReset_triggered() {
    ui->packetView->resetAll();
}

void MainWindow::on_actionLog_triggered(bool checked) {
    if (checked) {
        view_group->setCurrent(ui->logWindow);
        enableActions(false);
        ui->actionLog->setEnabled(true);
        ui->actionInfo->setEnabled(true);
        ui->actionSettings->setEnabled(true);
    }
    else {
        view_group->setCurrent(ui->splitter);
        enableActions(true);
    }
}

void MainWindow::on_actionInfo_triggered(bool checked) {
    if (checked) {
        view_group->setCurrent(ui->infoBrowser);
        enableActions(false);
        ui->actionLog->setEnabled(true);
        ui->actionInfo->setEnabled(true);
        ui->actionSettings->setEnabled(true);
    }
    else {
        view_group->setCurrent(ui->splitter);
        enableActions(true);
    }
}

void MainWindow::onRowChoosingStateChanged(const QModelIndex & /*index*/) {
    ui->actionApply->setEnabled(actionApplyState());
    ui->actionReset->setEnabled(actionApplyState());
    if (searchWidget->filter() == FilterToolButton::IS_MARKED) ui->packetView->refreshRows("",AlpmPackage::NAME,AlpmPackage::IS_MARKED,"","");
}

void MainWindow::logString(const QString & str) {
    ui->logWindow->appendPlainText(str.endsWith("\n")?str.left(str.length()-1):str);
    ui->logWindow->ensureCursorVisible();
}

void MainWindow::on_actionRefresh_triggered() {
    enableActions(false);

    if (optdep != nullptr) optdep->deleteLater();
    view_group->setCurrent(ui->progressView);
    ui->actionCancel->setVisible(true);
    ui->actionApply->setVisible(false);
    ui->packetView->clear();
    ui->progressView->clear();
    PackageProcessor * processor = new DBRefresher(ui->progressView,ui->actionCancel);
    connect(processor,&PackageProcessor::completed,this,&MainWindow::dbRefreshCompleted,Qt::QueuedConnection);
    connect(processor,&PackageProcessor::logString,this,&MainWindow::logString);
}

void MainWindow::dbRefreshCompleted(ThreadRun::RC rc,const QString &) {
    if (rc == ThreadRun::OK || rc == ThreadRun::TERMINATED || rc == ThreadRun::FORBIDDEN) {
        ui->actionCancel->setVisible(false);
        ui->actionApply->setVisible(true);
        enableActions(true);
        initCombos();
        QMetaObject::invokeMethod(ui->packetView,"refreshRows",Qt::QueuedConnection);
    }
    else {
        ui->actionCancel->setEnabled(true);
        connect(ui->actionCancel,&QAction::triggered,this,&MainWindow::error_cancel_triggered);
    }
}

void MainWindow::error_cancel_triggered(bool do_refresh) {
    disconnect(ui->actionCancel,&QAction::triggered,this,&MainWindow::error_cancel_triggered);
    ui->actionCancel->setEnabled(false);
    ui->actionCancel->setVisible(false);
    ui->actionApply->setVisible(true);
    enableActions(true);
    if (do_refresh) ui->packetView->refreshRows();
    else view_group->setCurrent(ui->splitter);
}

void MainWindow::showEvent(QShowEvent * event) {
    QMainWindow::showEvent(event);
    if (!wasInit) {
        wasInit = true;
        QSettings settings;
        restoreGeometry(settings.value("geometry/mainwindow").toByteArray());
        restoreState(settings.value("state/mainwindow").toByteArray());
        ui->splitter->restoreState(settings.value("state/splitter").toByteArray());
    }
}

void MainWindow::closeEvent(QCloseEvent * event) {
    QSettings settings;
    settings.setValue("geometry/mainwindow",saveGeometry());
    settings.setValue("state/mainwindow",saveState());
    settings.setValue("state/splitter",ui->splitter->saveState());

    QMainWindow::closeEvent(event);
}

void MainWindow::onActionApply() {
    enableActions(false);

    if (optdep != nullptr) delete optdep;
    view_group->setCurrent(ui->progressView);
    ui->actionCancel->setVisible(true);
    ui->actionApply->setVisible(false);
    ui->packetView->clear();
    ui->progressView->clear();
    optdep = new OptionalDepsDlg();
    ActionApplier * applier = new ActionApplier(ui->progressView,ui->actionCancel,optdep);
    connect(optdep,&OptionalDepsDlg::selected,this,&MainWindow::onselect_packages);
    connect(optdep,&QObject::destroyed,this,[&]() { optdep = nullptr; });
    connect(applier,&ActionApplier::completed,this,&MainWindow::onApplierCompleted,Qt::QueuedConnection);
    connect(applier,&ActionApplier::logString,this,&MainWindow::logString);
}

void MainWindow::onselect_packages(const QStringList & pkgs) {
    if (pkgs.count() <= 0) return;

    for (int i=0;i<pkgs.count();i++) {
        ui->packetView->markPackageByNameToInstall(pkgs.at(i));
    }

    ui->packetView->refreshRows("",AlpmPackage::NAME,AlpmPackage::IS_MARKED,"","");
}

void MainWindow::onApplierCompleted(ThreadRun::RC rc,const QString &) {
    if (rc == ThreadRun::OK || rc == ThreadRun::TERMINATED || rc == ThreadRun::FORBIDDEN) {
        ui->actionCancel->setVisible(false);
        ui->actionApply->setVisible(true);
        enableActions(true);
        if (optdep != nullptr) optdep->setProcessingEnabled();
        ui->packetView->refreshRows();
    }
    else {
        if (optdep != nullptr) optdep->deleteLater();
        ui->actionCancel->setEnabled(true);
        connect(ui->actionCancel,&QAction::triggered,this,&MainWindow::error_cancel_triggered);
    }
}

void MainWindow::on_actionFullUpdate_triggered() {
    enableActions(false);

    if (optdep != nullptr) delete optdep;
    view_group->setCurrent(ui->progressView);
    ui->actionCancel->setVisible(true);
    ui->actionApply->setVisible(false);
    ui->packetView->clear();
    ui->progressView->clear();
    optdep = new OptionalDepsDlg();
    PackageInstaller * installer = new PackageInstaller(QList<AlpmPackage>(),QList<AlpmPackage>(),false,ui->progressView,ui->actionCancel,optdep);
    connect(optdep,&OptionalDepsDlg::selected,this,&MainWindow::onselect_packages);
    connect(optdep,&QObject::destroyed,this,[&]() { optdep = nullptr; });
    connect(installer,&PackageInstaller::completed,this,&MainWindow::onApplierCompleted,Qt::QueuedConnection);
    connect(installer,&PackageInstaller::logString,this,&MainWindow::logString);
}

void MainWindow::onDownloadRequested(const AlpmPackage & pkg) {
    enableActions(false);

    if (optdep != nullptr) delete optdep;
    view_group->setCurrent(ui->progressView);
    ui->actionCancel->setVisible(true);
    ui->actionApply->setVisible(false);
    PackageProcessor * processor = new PackageDownloader(QList<AlpmPackage>() << pkg,ui->progressView,ui->actionCancel);
    connect(processor,&PackageProcessor::completed,this,&MainWindow::onDownloadingCompleted,Qt::QueuedConnection);
    connect(processor,&PackageProcessor::logString,this,&MainWindow::logString);
}

void MainWindow::onDownloadingCompleted(ThreadRun::RC rc,const QString &) {
    if (rc == ThreadRun::OK || rc == ThreadRun::TERMINATED || rc == ThreadRun::FORBIDDEN) {
        ui->actionCancel->setVisible(false);
        ui->actionApply->setVisible(true);
        enableActions(true);
        view_group->setCurrent(ui->splitter);
        if (rc == ThreadRun::OK) ui->filesTree->refill();
    }
    else {
        ui->actionCancel->setEnabled(true);
        connect(ui->actionCancel,&QAction::triggered,this,[&]() {error_cancel_triggered(false);});
    }
}

void MainWindow::onTrayRefresherStart() {
    on_actionRefresh_triggered();
}

void MainWindow::onTrayUpdaterStart() {
    on_actionFullUpdate_triggered();
}

void MainWindow::on_actionSettings_triggered(bool checked) {
    if (checked) {
        view_group->setCurrent(ui->settingsBrowser);
        enableActions(false);
        ui->actionLog->setEnabled(true);
        ui->actionInfo->setEnabled(true);
        ui->actionSettings->setEnabled(true);
    }
    else {
        view_group->setCurrent(ui->splitter);
        enableActions(true);
    }
}

