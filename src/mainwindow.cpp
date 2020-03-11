/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "searchwidget.h"
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <QActionGroup>
#include <QProcessEnvironment>
#include "static.h"
#include "messagedialog.h"
#include "dbrefresher.h"
#include "packageinstaller.h"
#include "packagedownloader.h"
#include "themeicons.h"
#include "actionapplier.h"
#include <sys/types.h>
#include <pwd.h>
#include "lockfilewaiter.h"
#include "exclusiveactiongroup.h"
#include "widgetgroup.h"
#include "alpmlockingnotifier.h"
#include "optionaldepsdlg.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow) {
    ui->setupUi(this);

    wasInit = false;
    need_alpm_reload = false;
    optdep = NULL;
    itemSelTimer.setInterval(200);

    view_group = new WidgetGroup(this);
    view_group->add(ui->splitter);
    view_group->add(ui->progressView);
    view_group->add(ui->waitView);
    view_group->add(ui->logWindow);
    view_group->add(ui->infoBrowser);

    QActionGroup * acgroup = new ExclusiveActionGroup(this);
    acgroup->addAction(ui->actionLog);
    acgroup->addAction(ui->actionInfo);

    ui->actionApply->setIcon(ThemeIcons::get(ThemeIcons::INSTALL));
    ui->actionReset->setIcon(ThemeIcons::get(ThemeIcons::UNDO_OR_RESET));
    ui->actionMark_All->setIcon(ThemeIcons::get(ThemeIcons::PKG_SELECT_ALL));
    ui->actionRefresh->setIcon(ThemeIcons::get(ThemeIcons::UPDATE_REPOS));
    ui->actionLog->setIcon(ThemeIcons::get(ThemeIcons::LOG_VIEW));
    ui->actionRefreshList->setIcon(ThemeIcons::get(ThemeIcons::REFRESH));
    ui->actionCacheCleanUp->setIcon(ThemeIcons::get(ThemeIcons::CLEAN_CACHE));
    ui->actionPrevious->setIcon(ThemeIcons::get(ThemeIcons::PREV));
    ui->actionNext->setIcon(ThemeIcons::get(ThemeIcons::NEXT));
    ui->actionFullUpdate->setIcon(ThemeIcons::get(ThemeIcons::SYNC));
    ui->actionCancel->setIcon(ThemeIcons::get(ThemeIcons::CANCEL));

    ui->actionApply->setVisible(getuid() == 0);
    ui->actionMark_All->setVisible(getuid() == 0);
    if (getuid() == 0) {
        ui->actionsToolBar->insertSeparator(ui->actionLog);
    }
    else setWindowTitle(windowTitle()+" ("+tr("readonly mode")+")");
    ui->actionReset->setVisible(getuid() == 0);
    ui->actionFullUpdate->setVisible(getuid() == 0);
    ui->actionRefresh->setVisible(getuid() == 0);

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

    lock_notifier = new AlpmLockingNotifier(this);

    connect(ui->actionApply,SIGNAL(triggered()),this,SLOT(onActionApply()));
    connect(searchWidget,SIGNAL(search_changed(const QString &,CategoryToolButton::ItemId,FilterToolButton::ItemId,const QString &,const QString &)),this,SLOT(onSearchChanged(const QString &,CategoryToolButton::ItemId,FilterToolButton::ItemId,const QString &,const QString &)));
    connect(ui->packetView,SIGNAL(refreshBeginning()),this,SLOT(refreshBeginning()));
    connect(ui->packetView,SIGNAL(refreshMaybeStarted(bool *)),this,SLOT(refreshMaybeStarted(bool *)));
    connect(ui->packetView,SIGNAL(selectionChanged(const QModelIndex &)),this,SLOT(onSelectionChanged(const QModelIndex &)));
    connect(ui->packetView,SIGNAL(refreshCompleted()),this,SLOT(onRefreshCompleted()));
    connect(ui->packetView,SIGNAL(rowChoosingStateChanged(const QModelIndex &)),this,SLOT(onRowChoosingStateChanged(const QModelIndex &)));
    connect(ui->pacInfoView,SIGNAL(groupUrlSelected(const QString &)),this,SLOT(onGroupUrlSelected(const QString &)));
    connect(ui->pacInfoView,SIGNAL(reasonUrlSelected(const QString &)),this,SLOT(onReasonUrlSelected(const QString &)));
    connect(ui->pacInfoView,SIGNAL(packageUrlSelected(const QString &,const QString &,int)),this,SLOT(onPackageUrlSelected(const QString &,const QString &,int)));
    connect(ui->filesTree,SIGNAL(downloadRequested(AlpmPackage *)),this,SLOT(onDownloadRequested(AlpmPackage *)));

    connect(lock_notifier,SIGNAL(unlocked()),this,SLOT(alpm_locking_changed()),Qt::QueuedConnection);
    connect(&itemSelTimer,SIGNAL(timeout()),SLOT(onSelectedItemTimeout()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onDownloadRequested(AlpmPackage * pkg) {
    enableActions(false);

    view_group->setCurrent(ui->progressView);
    ui->actionCancel->setVisible(true);
    ui->actionApply->setVisible(false);
    PackageProcessor * processor = new PackageDownloader(QList<AlpmPackage *>() << pkg,ui->progressView,ui->actionCancel);
    connect(processor,SIGNAL(completed(ThreadRun::RC)),SLOT(blockedDownloadingCompleted(ThreadRun::RC)),Qt::QueuedConnection);
    connect(processor,SIGNAL(logString(const QString &)),this,SLOT(logString(const QString &)));
}

void MainWindow::refreshMaybeStarted(bool * ok) {
    QString userSyncDir = qgetenv("QPACMAN_SYNC_DIR");
    if (Alpm::isOpen() && !userSyncDir.isEmpty() && QDir(userSyncDir).exists() && Static::isLeftDirNewer(QDir(userSyncDir),QDir(Alpm::instance()->dbDirPath()+QDir::separator()+QString::fromLocal8Bit("sync")))) {
        updateSystemSyncDir(QDir(Alpm::instance()->dbDirPath()+QDir::separator()+QString::fromLatin1("sync")),QDir(userSyncDir));
    }
    *ok = true;
}

void MainWindow::updateSystemSyncDir(const QDir & sys_sync_path,const QDir & user_sync_path) {
    QStringList lock_files_to_wait;
    QString sys_lock_file = sys_sync_path.path()+QDir::separator()+".." + QDir::separator() + "db.lck";
    if (QFile(sys_lock_file).exists()) lock_files_to_wait.append(sys_lock_file);
    QString user_lock_file = user_sync_path.path()+QDir::separator()+".." + QDir::separator() + "db.lck";
    if (QFile(user_lock_file).exists()) lock_files_to_wait.append(user_lock_file);
    if (lock_files_to_wait.count() > 0) {
        if (LockFileWaiter(lock_files_to_wait).exec() == QDialog::Rejected) {
            QCoreApplication::exit(100);
            return;
        }
    }
    QFile sys_lock(sys_lock_file);
    if (sys_lock.open(QIODevice::WriteOnly)) sys_lock.close();
    else {
        qDebug() << "Error:" << "something wrong: cannot create lock file:" << sys_lock_file;
        QCoreApplication::exit(101);
        return;
    }
    QFile user_lock(user_lock_file);
    if (user_lock.open(QIODevice::WriteOnly)) {
        user_lock.close();
        QString orig_user = QProcessEnvironment::systemEnvironment().value("ORIGINAL_USER","");
        if (orig_user.isEmpty()) {
            qDebug() << "Error:" << "something wrong with application setup, ask programmer what to do";
            QCoreApplication::exit(103);
            return;
        }
        struct passwd * pw = getpwnam(orig_user.toLocal8Bit().constData());
        if (pw == NULL) {
            qDebug() << "Error:" << "something wrong with application setup, ask programmer what to do";
            QCoreApplication::exit(104);
            return;
        }
        chown(user_lock_file.toLocal8Bit().constData(),pw->pw_uid,pw->pw_gid);
    }
    else {
        qDebug() << "Error:" << "something wrong: cannot create user lock file:" << user_lock_file;
        QCoreApplication::exit(102);
        return;
    }
    bool ret = copyDirectoryFiles(user_sync_path.path(),sys_sync_path.path(),"db");
    user_lock.remove();
    sys_lock.remove();
    if (!ret) {
        QCoreApplication::exit(102);
        return;
    }
    QMetaObject::invokeMethod(this,"blockedOperationCompleted",Qt::QueuedConnection,Q_ARG(ThreadRun::RC,ThreadRun::OK));
}

bool MainWindow::copyDirectoryFiles(const QString & fromDir,const QString & toDir,const QString & suffix) {
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    if(!targetDir.exists() && !targetDir.mkdir(targetDir.absolutePath())) {
        qDebug() << "Error:" << "cannot create dir:" << targetDir;
        return false;
    }
    if(!sourceDir.exists()) {
        qDebug() << "Error:" << "source dir does not exist:" << fromDir;
        return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach (QFileInfo fileInfo, fileInfoList) {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..") continue;

        if(fileInfo.isDir()) {
            if(!copyDirectoryFiles(fileInfo.filePath(),targetDir.filePath(fileInfo.fileName()))) return false;
        }
        else {
            if (!suffix.isEmpty() && fileInfo.suffix() != suffix) continue;
            if(targetDir.exists(fileInfo.fileName())) targetDir.remove(fileInfo.fileName());
            if(!QFile::copy(fileInfo.filePath(),targetDir.filePath(fileInfo.fileName()))) {
                qDebug() << "Error:" << "cannot copy" << fileInfo.filePath() << "to" << targetDir.filePath(fileInfo.fileName());
                return false;
            }
        }
    }

    need_alpm_reload = true;

    return true;
}

void MainWindow::alpm_locking_changed() {
    if (ui->packetView->isRefreshing()) return;
    need_alpm_reload = true;
    ui->actionRefreshList->trigger();
}

void MainWindow::onSearchChanged(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo) {
    ui->packetView->applyFilter(text,cItemId,fItemId,group,repo);
}

void MainWindow::onSelectionChanged(const QModelIndex & index) {
    if (itemSelTimer.isActive()) itemSelTimer.stop();
    itemSelTimer.setProperty("index",index);
    itemSelTimer.start();
}

void MainWindow::onSelectedItemTimeout() {
    QModelIndex index = itemSelTimer.property("index").toModelIndex();

    AlpmPackage * pkg = ui->packetView->row(index);
    if (pkg == NULL) return;
    ui->pacInfoView->fillByInfo(pkg);
    ui->filesTree->fill(ui->packetView->row(index));
    ui->actionNext->setEnabled(ui->packetView->isSelectNextPossible());
    ui->actionPrevious->setEnabled(ui->packetView->isSelectPrevPossible());
    itemSelTimer.stop();
}

void MainWindow::onGroupUrlSelected(const QString & group) {
    searchWidget->setFilter(FilterToolButton::IS_GROUP,group);
    searchWidget->clearSearchText();
}

void MainWindow::onPackageUrlSelected(const QString & name,const QString & version,int operation) {
    searchWidget->clearSearchText();
    searchWidget->clearFilter();
    ui->packetView->selectPackage(AlpmPackage::Dependence(name,(AlpmPackage::CompareOper)operation,version));
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

void MainWindow::on_actionMark_All_triggered() {
    ui->packetView->markAll();
}

void MainWindow::on_actionReset_triggered() {
    ui->packetView->resetAll();
}

void MainWindow::removePackageIfExistInList(const QStringList & packages,QStringList & in_out_packages) {
    for (int i=0;i<packages.count();i++) {
        if (in_out_packages.contains(packages.at(i))) in_out_packages.removeAll(packages.at(i));
    }
}

void MainWindow::onActionApply() {
    lock_notifier->blockSignals(true);
    QString lockfile = Alpm::instance()->lockFilePath();
    if (QFile(lockfile).exists() && LockFileWaiter(QStringList() << lockfile).exec() == QDialog::Rejected) return;
    lock_notifier->blockSignals(false);

    enableActions(false);

    QList<AlpmPackage *> to_remove;
    QList<AlpmPackage *> to_removeall;
    QList<AlpmPackage *> to_install;
    QList<AlpmPackage *> to_installdeps;
    QList<AlpmPackage *> to_installforced;
    ui->packetView->markedPackages(to_install,to_installdeps,to_installforced,to_removeall,to_remove);

    view_group->setCurrent(ui->progressView);
    ui->actionCancel->setVisible(true);
    ui->actionApply->setVisible(false);
    optdep = new OptionalDepsDlg();
    ActionApplier * applier = new ActionApplier(to_removeall,to_remove,to_install,to_installdeps,to_installforced,ui->progressView,ui->actionCancel,optdep);
    connect(optdep,SIGNAL(selected(const QStringList &)),this,SLOT(onselect_packages(const QStringList &)));
    connect(optdep,&QObject::destroyed,this,[&]() { optdep = NULL; });
    connect(applier,SIGNAL(completed(ThreadRun::RC)),this,SLOT(blockedOperationCompleted(ThreadRun::RC)),Qt::QueuedConnection);
    connect(applier,SIGNAL(logString(const QString &)),this,SLOT(logString(const QString &)));
}

void MainWindow::onselect_packages(const QStringList & pkgs) {
    for (int i=0;i<pkgs.count();i++) {
        ui->packetView->markPackageByNameToInstall(pkgs.at(i));
    }

    if (pkgs.count() > 0) searchWidget->setFilter(FilterToolButton::IS_MARKED,"");
}

void MainWindow::on_actionRefresh_triggered() {
    lock_notifier->blockSignals(true);
    QString lockfile = Alpm::instance()->lockFilePath();
    if (QFile(lockfile).exists() && LockFileWaiter(QStringList() << lockfile).exec() == QDialog::Rejected) return;
    lock_notifier->blockSignals(false);

    enableActions(false);

    view_group->setCurrent(ui->progressView);
    ui->actionCancel->setVisible(true);
    ui->actionApply->setVisible(false);
    PackageProcessor * processor = new DBRefresher(ui->progressView,ui->actionCancel);
    connect(processor,SIGNAL(completed(ThreadRun::RC)),SLOT(blockedOperationCompleted(ThreadRun::RC)),Qt::QueuedConnection);
    connect(processor,SIGNAL(logString(const QString &)),this,SLOT(logString(const QString &)));
}

void MainWindow::on_actionFullUpdate_triggered() {
    lock_notifier->blockSignals(true);
    QString lockfile = Alpm::instance()->lockFilePath();
    if (QFile(lockfile).exists() && LockFileWaiter(QStringList() << lockfile).exec() == QDialog::Rejected) return;
    lock_notifier->blockSignals(false);

    enableActions(false);

    view_group->setCurrent(ui->progressView);
    ui->actionCancel->setVisible(true);
    ui->actionApply->setVisible(false);
    optdep = new OptionalDepsDlg();
    PackageInstaller * installer = new PackageInstaller(QList<AlpmPackage *>(),QList<AlpmPackage *>(),false,ui->progressView,ui->actionCancel,optdep);
    connect(optdep,SIGNAL(selected(const QStringList &)),this,SLOT(onselect_packages(const QStringList &)));
    connect(optdep,&QObject::destroyed,this,[&]() { optdep = NULL; });
    connect(installer,SIGNAL(completed(ThreadRun::RC)),SLOT(blockedOperationCompleted(ThreadRun::RC)),Qt::QueuedConnection);
    connect(installer,SIGNAL(logString(const QString &)),this,SLOT(logString(const QString &)));
}

void MainWindow::blockedOperationCompleted(ThreadRun::RC rc) {
    ActionApplier * applier = qobject_cast<ActionApplier *>(QObject::sender());
    DBRefresher * refresher = qobject_cast<DBRefresher *>(QObject::sender());

    if (rc != ThreadRun::BAD && rc != ThreadRun::FORBIDDEN) {
        bool isPartiallyOK = (rc == ThreadRun::TERMINATED) && ((applier != NULL)?applier->isPartiallyOK():false);
        ui->actionCancel->setVisible(false);
        ui->actionApply->setVisible(getuid() == 0);
        enableActions(true);
        view_group->setCurrent(ui->splitter);
        if (rc != ThreadRun::TERMINATED || isPartiallyOK) {
            ui->packetView->refresh(refresher != NULL);
            if (optdep != NULL) optdep->setProcessingEnabled();
        }
        else if (optdep != NULL) optdep->deleteLater();
    }
    else {
        if (optdep != NULL) optdep->deleteLater();
        ui->actionCancel->setEnabled(true);
        connect(ui->actionCancel,SIGNAL(triggered()),this,SLOT(error_cancel_triggered()));
    }
}

void MainWindow::error_cancel_triggered() {
    disconnect(ui->actionCancel,SIGNAL(triggered()),this,SLOT(error_cancel_triggered()));
    ui->actionCancel->setEnabled(false);
    ui->actionCancel->setVisible(false);
    ui->actionApply->setVisible(getuid() == 0);
    enableActions(true);
    view_group->setCurrent(ui->splitter);
}

void MainWindow::blockedDownloadingCompleted(ThreadRun::RC rc) {
    if (rc != ThreadRun::BAD && rc != ThreadRun::FORBIDDEN) {
        enableActions(true);
        view_group->setCurrent(ui->splitter);
        ui->actionCancel->setVisible(false);
        ui->actionApply->setVisible(getuid() == 0);
        ui->filesTree->refill();
    }
    else {
        ui->actionCancel->setEnabled(true);
        connect(ui->actionCancel,SIGNAL(triggered()),this,SLOT(error_cancel_triggered()));
    }
}

void MainWindow::on_actionRefreshList_triggered(bool) {
    ui->packetView->refresh();
}

void MainWindow::on_actionCacheCleanUp_triggered() {
    if (QMessageBox::warning(this,Static::Warning_Str,tr("The contents of cache directory will be removed.\nAre you sure to continue?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::No) return;
    if (Alpm::instance()->cleanCacheDirs()) MessageDialog::post(tr("The contents of cache directory was removed succesfully!"),QString(),Static::Info_Str);
    else ErrorDialog::post(tr("The contents of cache directory was not removed!"),Alpm::instance()->lastError(),Static::Error_Str);
}

void MainWindow::onReasonUrlSelected(const QString & package) {
    ui->packetView->revertReason(package);
    ui->packetView->refresh();
}

void MainWindow::onRowChoosingStateChanged(const QModelIndex & /*index*/) {
    ui->actionApply->setEnabled(actionApplyState());
    ui->actionReset->setEnabled(actionApplyState());
    if (searchWidget->filter() == FilterToolButton::IS_MARKED) searchWidget->setFilter(FilterToolButton::IS_MARKED,"");
}

void MainWindow::onRefreshCompleted() {
    SearchWidget * sw = ui->mainToolBar->findSearchWidget();
    sw->fillFiltersInCombo(Alpm::instance()->groups());
    sw->fillReposInCombo(Alpm::instance()->repos());

    onRowChoosingStateChanged(QModelIndex());

    ui->filesTree->clear();
    ui->pacInfoView->clear();

    QMetaObject::invokeMethod(this,"setFilter",Qt::QueuedConnection);
}

void MainWindow::setFilter() {
    enableActions(true);
    view_group->setCurrent(ui->splitter);
}

void MainWindow::enableActions(bool flag) {
    QList<QAction *> actions = Static::childrenActions(this);
    for (int i=0;i<actions.count();i++) {
        actions[i]->setEnabled(flag);
    }
    ui->actionApply->setEnabled(!flag?false:actionApplyState());
    ui->actionReset->setEnabled(!flag?false:actionApplyState());
    ui->actionNext->setEnabled(!flag?false:ui->packetView->isSelectNextPossible());
    ui->actionPrevious->setEnabled(!flag?false:ui->packetView->isSelectPrevPossible());
}

void MainWindow::refreshBeginning() {
    enableActions(false);
    view_group->setCurrent(ui->waitView);

    if (need_alpm_reload) {
        need_alpm_reload = false;
        if (!Alpm::instance()->reopen()) qCritical("Cannot reload Alpm!!!");
    }
}

bool MainWindow::actionApplyState() {
    QList<AlpmPackage *> to_remove;
    QList<AlpmPackage *> to_removeall;
    QList<AlpmPackage *> to_install;
    QList<AlpmPackage *> to_installdeps;
    QList<AlpmPackage *> to_installforced;
    ui->packetView->markedPackages(to_install,to_installdeps,to_installforced,to_removeall,to_remove);
    return ((to_removeall.count() > 0) || (to_remove.count() > 0) || (to_install.count() > 0) || (to_installdeps.count() > 0));
}

void MainWindow::on_actionPrevious_triggered() {
    ui->packetView->selectPrev();
}

void MainWindow::on_actionNext_triggered() {
    ui->packetView->selectNext();
}

void MainWindow::on_actionLog_triggered(bool checked) {
    if (checked) {
        view_group->setCurrent(ui->logWindow);
        enableActions(false);
        ui->actionLog->setEnabled(true);
        ui->actionInfo->setEnabled(true);
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
    }
    else {
        view_group->setCurrent(ui->splitter);
        enableActions(true);
    }
}

void MainWindow::logString(const QString & str) {
    ui->logWindow->appendPlainText(str.endsWith("\n")?str.left(str.length()-1):str);
    ui->logWindow->ensureCursorVisible();
}
