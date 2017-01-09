/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "searchwidget.h"
#include <QSettings>
#include <QMessageBox>
#include "dbrefreshdialog.h"
#include "removeprogressloop.h"
#include "installprogressloop.h"
#include "logwindow.h"
#include "toolbarrightwidget.h"
#include "pacmancachecleaner.h"
#include "static.h"


MainWindow::MainWindow(bool startsOnUpdates,QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow) {
    log_window = new LogWindow();

    m_startsOnUpdates = startsOnUpdates;

    ui->setupUi(this);

    wasInit = false;

    ui->packetView->setWaitView(ui->waitView);
    ui->waitView->setVisible(false);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addWidget((searchWidget = new SearchWidget(ui->mainToolBar)));
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addWidget((rightWidget = new ToolbarRightWidget(ui->mainToolBar)));
    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->actionsToolBar->insertWidget(ui->actionApply,empty);
    ui->actionLog->setVisible(false);
    ui->actionApply->setEnabled(false);

    connect(log_window,SIGNAL(hidden()),this,SLOT(onLogWindowHidden()));
    connect(searchWidget,SIGNAL(search_changed(const QString &,CategoryToolButton::ItemId,FilterToolButton::ItemId,const QString &,const QString &)),this,SLOT(onSearchChanged(const QString &,CategoryToolButton::ItemId,FilterToolButton::ItemId,const QString &,const QString &)));
    connect(ui->packetView,SIGNAL(enableActions(bool)),this,SLOT(onEnableActions(bool)));
    connect(ui->packetView,SIGNAL(canFillFilters(const QStringList &)),this,SLOT(onCanFillFiltersInCombo(const QStringList &)));
    connect(ui->packetView,SIGNAL(canFillRepos(const QStringList &)),this,SLOT(onCanFillReposInCombo(const QStringList &)));
    connect(ui->packetView,SIGNAL(selectionChanged(const QModelIndex &)),this,SLOT(onSelectionChanged(const QModelIndex &)));
    connect(ui->packetView,SIGNAL(refreshCompleted()),this,SLOT(onRefreshCompleted()));
    connect(ui->packetView,SIGNAL(rowChoosingStateChanged(const QModelIndex &)),this,SLOT(onRowChoosingStateChanged(const QModelIndex &)));
    connect(ui->pacInfoView,SIGNAL(groupUrlSelected(const QString &)),this,SLOT(onGroupUrlSelected(const QString &)));
    connect(ui->pacInfoView,SIGNAL(reasonUrlSelected(const QString &)),this,SLOT(onReasonUrlSelected(const QString &)));
    connect(ui->pacInfoView,SIGNAL(packageUrlSelected(const QString &)),this,SLOT(onPackageUrlSelected(const QString &)));
}

MainWindow::~MainWindow() {
    delete ui;
    delete log_window;
}

void MainWindow::onSearchChanged(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo) {
    ui->packetView->onSearchChanged(text,cItemId,fItemId,group,repo);
}

void MainWindow::onLogWindowHidden() {
    ui->actionLog->setChecked(false);
}

void MainWindow::onCanFillFiltersInCombo(const QStringList & groups) {
    ((SearchWidget *)ui->mainToolBar->findWidget(0))->fillFiltersInCombo(groups);
}

void MainWindow::onCanFillReposInCombo(const QStringList & repos) {
    ((SearchWidget *)ui->mainToolBar->findWidget(0))->fillReposInCombo(repos);
}

void MainWindow::onSelectionChanged(const QModelIndex & index) {
    ui->pacInfoView->clearImageCache();
    ui->pacInfoView->setHtml(ui->packetView->row(index).toHtml());
    PacmanEntry row = ui->packetView->row(index);
    if (row.isInstalled()) {
        ui->filesTree->fill(row.listFiles());
    }
    else {
        ui->filesTree->clear();
    }

    ui->actionNext->setEnabled(ui->packetView->isSelectNextPossible());
    ui->actionPrevious->setEnabled(ui->packetView->isSelectPrevPossible());
}

void MainWindow::onGroupUrlSelected(const QString & group) {
    searchWidget->setFilter(FilterToolButton::IS_GROUP,group);
    searchWidget->clearSearchText();
}

void MainWindow::onPackageUrlSelected(const QString & package) {
    searchWidget->clearSearchText();
    searchWidget->clearFilter();
    ui->packetView->selectPackageByName(package);
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

class TempWinDisable {
public:
    TempWinDisable(QWidget * widget) {
        m_widget = widget;
        m_widget->setEnabled(false);
    }
    ~TempWinDisable() {
        m_widget->setEnabled(true);
    }
private:
    QWidget * m_widget;
};


void MainWindow::removePackageIfExistInList(const QStringList & packages,QStringList & in_out_packages) {
    for (int i=0;i<packages.count();i++) {
        if (in_out_packages.contains(packages.at(i))) in_out_packages.removeAll(packages.at(i));
    }
}

void MainWindow::on_actionApply_triggered() {
    TempWinDisable temp(this);

    if (!Static::checkRootAccess()) {
        QMessageBox::critical(this,Static::Error_Str,Static::RootRightsNeeded_Str,QMessageBox::Ok);
        return;
    }

    QList<PacmanEntry> to_remove = ui->packetView->markedPackagesToRemove();
    QList<PacmanEntry> to_removeall = ui->packetView->markedPackagesToRemoveAll();
    QList<PacmanEntry> to_install = ui->packetView->markedPackagesToInstall();

    QStringList removed_packages;
    if (to_removeall.count() > 0) {
        RemoveProgressLoop rprogress_dlg(Static::su_password,PacmanEntry::entriesListToNamesStringList(to_removeall),true,this);
        connect(&rprogress_dlg,SIGNAL(post_messages(const QString &,const QStringList &)),this,SLOT(add_post_messages(const QString &,const QStringList &)));
        if (rprogress_dlg.exec() != QDialog::Accepted) return;
        removed_packages = rprogress_dlg.removedPackages();
    }

    QStringList remove_packages;
    if (to_remove.count() > 0) {
        remove_packages = PacmanEntry::entriesListToNamesStringList(to_remove);
        if (removed_packages.count() > 0) removePackageIfExistInList(removed_packages,remove_packages);
        if (remove_packages.count() > 0) {
            RemoveProgressLoop rprogress_dlg(Static::su_password,remove_packages,false,this);
            connect(&rprogress_dlg,SIGNAL(post_messages(const QString &,const QStringList &)),this,SLOT(add_post_messages(const QString &,const QStringList &)));
            if (rprogress_dlg.exec() != QDialog::Accepted) return;
        }
    }
    if (to_install.count() > 0) {
        InstallProgressLoop iprogress_dlg(Static::su_password,PacmanEntry::entriesListToStringList(to_install),this);
        connect(&iprogress_dlg,SIGNAL(post_messages(const QString &,const QStringList &)),this,SLOT(add_post_messages(const QString &,const QStringList &)));
        if (iprogress_dlg.exec() != QDialog::Accepted) {
            if (remove_packages.count() <= 0 &&
                to_removeall.count() <= 0) return;
        }
    }

    QMetaObject::invokeMethod(this,"on_actionRefreshList_triggered",Qt::QueuedConnection);
}

void MainWindow::add_post_messages(const QString & package,const QStringList & messages) {
    ui->actionLog->setVisible(true);
    log_window->addText(Static::PostMessages_Str.arg(package));
    log_window->addText("");
    log_window->addText(messages.join("\n"));
    log_window->addText("");
}

void MainWindow::on_actionRefresh_triggered() {
    DBRefreshDialog(this).exec();
    on_actionRefreshList_triggered();
}

void MainWindow::on_actionLog_triggered(bool checked) {
    log_window->setVisible(checked);
}

void MainWindow::on_actionRefreshList_triggered() {
    ui->filesTree->clear();
    ui->pacInfoView->clear();
    ui->packetView->refresh();
}

void MainWindow::on_actionCacheCleanUp_triggered() {
    if (!Static::checkRootAccess()) {
        QMessageBox::critical(this,Static::Error_Str,Static::RootRightsNeeded_Str,QMessageBox::Ok);
        return;
    }

    if (QMessageBox::warning(this,Static::Warning_Str,tr("The contents of cache directory will be removed.\nAre you sure to continue?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::No) return;
    PacmanCacheCleaner cleaner(Static::su_password);
    cleaner.waitToComplete();
    if (cleaner.exitCode() != 0) return;
    QMessageBox::information(this,"Information...",tr("The contents of cache directory was removed succesfully!"));
}

void MainWindow::onReasonUrlSelected(const QString & package) {
    ui->packetView->revertReason(package);
    ui->packetView->refresh();
}

void MainWindow::onRowChoosingStateChanged(const QModelIndex & /*index*/) {
    ui->actionApply->setEnabled(actionApplyState());
    if (searchWidget->filter() == FilterToolButton::IS_MARKED) searchWidget->setFilter(FilterToolButton::IS_MARKED,"");
}

void MainWindow::onRefreshCompleted() {
    onRowChoosingStateChanged(QModelIndex());

    ui->pacInfoView->setModel((PacmanItemModel *)((QAbstractItemView *)ui->packetView)->model());

    ui->filesTree->clear();
    ui->pacInfoView->clear();

    if (m_startsOnUpdates) {
        searchWidget->setFilter(FilterToolButton::IS_NEEDUPDATE,"");
        on_actionMark_All_triggered();
        m_startsOnUpdates = false;
    }
}

void MainWindow::onEnableActions(bool flag) {
    QList<QAction *> actions = Static::childrenActions(this);
    for (int i=0;i<actions.count();i++) {
        actions[i]->setEnabled(flag);
    }
    ui->actionApply->setEnabled(!flag?false:actionApplyState());
    ui->actionNext->setEnabled(!flag?false:ui->packetView->isSelectNextPossible());
    ui->actionPrevious->setEnabled(!flag?false:ui->packetView->isSelectPrevPossible());
}

bool MainWindow::actionApplyState() {
    QList<PacmanEntry> to_removeall = ui->packetView->markedPackagesToRemoveAll();
    QList<PacmanEntry> to_remove = ui->packetView->markedPackagesToRemove();
    QList<PacmanEntry> to_install = ui->packetView->markedPackagesToInstall();
    return ((to_removeall.count() > 0) || (to_remove.count() > 0) || (to_install.count() > 0));
}

void MainWindow::on_actionPrevious_triggered() {
    ui->packetView->selectPrev();
}

void MainWindow::on_actionNext_triggered() {
    ui->packetView->selectNext();
}
