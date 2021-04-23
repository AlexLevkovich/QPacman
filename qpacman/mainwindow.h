/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "alpmpackage.h"
#include "categorytoolbutton.h"
#include "filtertoolbutton.h"
#include "qalpmtypes.h"

class WidgetGroup;
class SearchWidget;
class OptionalDepsDlg;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRefreshBeginning();
    void onRefreshCompleted();
    void onViewSelectionChanged(const AlpmPackage & pkg);
    void onReasonUrlSelected(const QString & pkgname);
    void onGroupUrlSelected(const QString & group);
    void onSearchChanged(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo);
    void onViewSearchChanged(const QString & text,AlpmPackage::SearchFieldType cItemId,AlpmPackage::PackageFilter fItemId,const QString & group,const QString & repo);
    void onPackageUrlSelected(const QString & name,const QString & version,int operation);
    void on_actionPrevious_triggered();
    void on_actionNext_triggered();
    void on_actionRefreshList_triggered(bool);
    void on_actionCacheCleanUp_triggered();
    void on_actionMark_All_triggered();
    void on_actionReset_triggered();
    void on_actionLog_triggered(bool checked);
    void on_actionInfo_triggered(bool checked);
    void onRowChoosingStateChanged(const QModelIndex & index);
    void on_actionRefresh_triggered();
    void logString(const QString & str);
    void dbRefreshCompleted(ThreadRun::RC rc,const QString & error);
    void error_cancel_triggered(bool do_refresh = true);
    void onActionApply();
    void onselect_packages(const QStringList & pkgs);
    void onApplierCompleted(ThreadRun::RC rc,const QString & error);
    void on_actionFullUpdate_triggered();
    void onDownloadRequested(const AlpmPackage & pkg);
    void onDownloadingCompleted(ThreadRun::RC rc,const QString & error);

protected:
    void closeEvent(QCloseEvent * event);
    void showEvent(QShowEvent * event);

private:
    void enableActions(bool flag);
    bool actionApplyState();
    void initCombos();

    Ui::MainWindow *ui;
    WidgetGroup * view_group;
    SearchWidget * searchWidget;
    bool wasInit;
    OptionalDepsDlg * optdep;
};
#endif // MAINWINDOW_H
