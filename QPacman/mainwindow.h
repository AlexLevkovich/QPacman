/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include "categorytoolbutton.h"
#include "filtertoolbutton.h"

namespace Ui {
class MainWindow;
}

class SearchWidget;
class LogWindow;
class ToolbarRightWidget;
class PacmanInstallPackagesReader;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(bool startsOnUpdates,QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void onCanFillFiltersInCombo(const QStringList & groups);
    void onCanFillReposInCombo(const QStringList & repos);
    void onSelectionChanged(const QModelIndex & index);
    void onGroupUrlSelected(const QString & group);
    void onPackageUrlSelected(const QString & package);
    void onReasonUrlSelected(const QString & package);

protected:
    void showEvent(QShowEvent * event);
    void closeEvent(QCloseEvent * event);
    
private slots:
    void on_actionMark_All_triggered();
    void on_actionReset_triggered();
    void on_actionApply_triggered();
    void on_actionRefresh_triggered();
    void on_actionLog_triggered(bool checked);
    void onLogWindowHidden();
    void add_post_messages(const QString & package,const QStringList & messages);
    void on_actionRefreshList_triggered();
    void on_actionCacheCleanUp_triggered();
    void onSearchChanged(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo);
    void onRowChoosingStateChanged(const QModelIndex & index);
    void onRefreshCompleted();
    void onEnableActions(bool flag);
    void dbus_loaded();
    void dbus_unloaded();

private:
    Ui::MainWindow *ui;
    SearchWidget * searchWidget;
    ToolbarRightWidget * rightWidget;
    bool wasInit;
    LogWindow * log_window;
    bool m_startsOnUpdates;

    bool actionApplyState();
    void removePackageIfExistInList(const QStringList & packages,QStringList & in_out_packages);
};

#endif // MAINWINDOW_H
