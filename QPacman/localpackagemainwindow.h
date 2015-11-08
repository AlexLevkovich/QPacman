/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef LOCALPACKAGEMAINWINDOW_H
#define LOCALPACKAGEMAINWINDOW_H

#include <QMainWindow>
#include <QItemSelection>

namespace Ui {
class LocalPackageMainWindow;
}

class PacmanSimpleItemModel;
class PacmanInstallPackagesReader;

class LocalPackageMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LocalPackageMainWindow(const QStringList & packages,QWidget *parent = 0);
    ~LocalPackageMainWindow();

protected slots:
    void onSelectionChanged(const QItemSelection & selected,const QItemSelection & deselected);
    void add_post_messages(const QString & package,const QStringList & messages);

private slots:
    void on_actionInstall_triggered();
    void init();
    void enableActions();
    void disableActions();
    void stop_wait_indicator();
    void start_wait_indicator();

private:
    Ui::LocalPackageMainWindow *ui;
    PacmanSimpleItemModel * model;
    QStringList m_packages;
    QString m_messages;
};

#endif // LOCALPACKAGEMAINWINDOW_H
