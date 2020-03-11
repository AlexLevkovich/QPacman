/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef LOCALPACKAGEMAINWINDOW_H
#define LOCALPACKAGEMAINWINDOW_H

#include <QMainWindow>
#include <QItemSelection>
#include "alpmfuture.h"

namespace Ui {
class LocalPackageMainWindow;
}

class PacmanSimpleItemModel;
class PacmanInstallPackagesReader;
class WidgetGroup;
class QCheckBox;

class LocalPackageMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LocalPackageMainWindow(const QStringList & packages,QWidget *parent = 0);
    ~LocalPackageMainWindow();

protected slots:
    void onSelectionChanged(const QItemSelection & selected,const QItemSelection & deselected);

private slots:
    void onActionInstall();
    void init();
    void operation_completed(ThreadRun::RC rc);
    void logString(const QString & str);
    void on_actionLog_triggered(bool checked);

private:
    Ui::LocalPackageMainWindow *ui;
    PacmanSimpleItemModel * model;
    QStringList m_packages;
    QString m_messages;
    WidgetGroup * view_group;
    QCheckBox * depsInstall;
    QCheckBox * forceInstall;
};

#endif // LOCALPACKAGEMAINWINDOW_H
