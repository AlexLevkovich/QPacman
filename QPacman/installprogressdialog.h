/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef INSTALLPROGRESSDIALOG_H
#define INSTALLPROGRESSDIALOG_H

#include <QProgressDialog>
#include "pacmaninstallpackagesreader.h"

class InstallProgressDialog : public QProgressDialog {
    Q_OBJECT
public:
    explicit InstallProgressDialog(const QStringList & packages,QWidget *parent = 0);

protected:
    explicit InstallProgressDialog(QWidget *parent = 0);
    virtual int packages_count();
    void showEvent(QShowEvent * event);

protected slots:
    void installing_packages_finished(PacmanProcessReader * reader);
    void start_installing(const QString & package);
    void start_removing(const QString & package);
    void start_download(const QString & url);
    void all_downloads_completed();
    void onCancel();
    void readyToProcess(double total_installed,double total_removed);
    void someProvidersAvailable(const QStringList & providers);
    void questionAvailable(const QString & question);
    void _hide();
    void onCancelDownload();

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
    int index;
    bool wasCanceled;
    bool canBeShown;

    void init();
};

#endif // INSTALLPROGRESSDIALOG_H
