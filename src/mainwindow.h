/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QDir>
#include <QTimer>
#include "categorytoolbutton.h"
#include "filtertoolbutton.h"
#include "alpmpackage.h"
#include "alpmfuture.h"

namespace Ui {
class MainWindow;
}

class SearchWidget;
class ToolbarRightWidget;
class PacmanInstallPackagesReader;
class PacmanProcessReader;
class LogWindow;
class WidgetGroup;
class OptionalDepsDlg;
class AlpmLockingNotifier;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void onSelectionChanged(const QModelIndex & index);
    void onGroupUrlSelected(const QString & group);
    void onPackageUrlSelected(const QString & name,const QString & version,int operation);
    void onReasonUrlSelected(const QString & package);

protected:
    void showEvent(QShowEvent * event);
    void closeEvent(QCloseEvent * event);
    
private slots:
    void onSelectedItemTimeout();
    void on_actionMark_All_triggered();
    void on_actionReset_triggered();
    void onActionApply();
    void on_actionRefresh_triggered();
    void on_actionFullUpdate_triggered();
    void on_actionRefreshList_triggered(bool ok);
    void on_actionCacheCleanUp_triggered();
    void onSearchChanged(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo);
    void onRowChoosingStateChanged(const QModelIndex & index);
    void onRefreshCompleted();
    void refreshBeginning();
    void on_actionPrevious_triggered();
    void on_actionNext_triggered();
    void setFilter();
    void blockedOperationCompleted(ThreadRun::RC rc);
    void blockedDownloadingCompleted(ThreadRun::RC rc);
    void alpm_locking_changed();
    void refreshMaybeStarted(bool * ok);
    void onDownloadRequested(AlpmPackage * pkg);
    void onselect_packages(const QStringList & pkgs);
    void error_cancel_triggered();
    void on_actionLog_triggered(bool checked);
    void on_actionInfo_triggered(bool checked);
    void logString(const QString & str);
    void updateSystemSyncDir(const QDir & sys_sync_path,const QDir & user_sync_path);

private:
    Ui::MainWindow *ui;
    WidgetGroup * view_group;
    SearchWidget * searchWidget;
    ToolbarRightWidget * rightWidget;
    QTimer itemSelTimer;
    bool wasInit;
    bool need_alpm_reload;
    OptionalDepsDlg * optdep;
    AlpmLockingNotifier * lock_notifier;

    bool copyDirectoryFiles(const QString & fromDir,const QString & toDir,const QString & suffix = QString());
    void enableActions(bool flag);
    bool actionApplyState();
    void removePackageIfExistInList(const QStringList & packages,QStringList & in_out_packages);

    friend class QPacmanApplication;
};

#endif // MAINWINDOW_H
