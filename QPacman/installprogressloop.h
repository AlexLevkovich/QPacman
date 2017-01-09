/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef INSTALLPROGRESSLOOP_H
#define INSTALLPROGRESSLOOP_H

#include <QEventLoop>
#include "pacmaninstallpackagesreader.h"


class InstallProgressLoop : public QEventLoop {
    Q_OBJECT
public:
    explicit InstallProgressLoop(const QString & su_password,const QStringList & packages,QWidget *parent = 0);

protected:
    explicit InstallProgressLoop(QWidget *parent = 0);
    virtual int packages_count();

protected slots:
    void installing_packages_finished(PacmanProcessReader * reader);
    void start_download(const QString & url);
    void all_downloads_completed();
    void onCancel();
    void readyToProcess(double total_installed,double total_removed);
    void someProvidersAvailable(const QStringList & providers);
    void questionAvailable(const QString & question);

signals:
    void post_messages(const QString & package_name,const QStringList & messages);
    void showingProvidersList();
    void hidingProvidersList();
    void showingPackageListDlg();
    void hidingPackageListDlg();
    void showingFilesDownloadDlg();
    void hidingFilesDownloadDlg();

protected:
    PacmanInstallPackagesReader * installer;

private:
    bool wasCanceled;
};

#endif // INSTALLPROGRESSDIALOG_H
