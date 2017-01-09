/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "installprogressloop.h"
#include "installprogressdialog.h"
#include "packagechangesdialog.h"
#include "filesdownloaddialog.h"
#include "static.h"
#include <QMessageBox>
#include <QKeyEvent>

InstallProgressLoop::InstallProgressLoop(const QString & su_password,const QStringList & packages,QWidget *parent) : QEventLoop(parent) {
    wasCanceled = false;
    installer = new PacmanInstallPackagesReader(su_password,packages.join(" "),this);
    connect(installer,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(installing_packages_finished(PacmanProcessReader *)));
    connect(installer,SIGNAL(post_messages(const QString &,const QStringList &)),this,SIGNAL(post_messages(const QString &,const QStringList &)));
    connect(installer,SIGNAL(ready_to_process(double,double)),this,SLOT(readyToProcess(double,double)));
    connect(installer,SIGNAL(some_providers_available(const QStringList &)),this,SLOT(someProvidersAvailable(const QStringList &)));
    connect(installer,SIGNAL(question_available(const QString &)),this,SLOT(questionAvailable(const QString &)));
    connect(installer,SIGNAL(start_download(const QString &)),this,SLOT(start_download(const QString &)));
    connect(installer,SIGNAL(all_downloads_completed()),this,SLOT(all_downloads_completed()),Qt::QueuedConnection);
}

InstallProgressLoop::InstallProgressLoop(QWidget *parent) : QEventLoop(parent) {
    wasCanceled = false;
    installer = NULL;
}

void InstallProgressLoop::all_downloads_completed() {
    emit hidingFilesDownloadDlg();
    int total = packages_count();
    InstallProgressDialog dlg(installer,(total == 1)?0:total,(QWidget *)parent());
    connect(&dlg,SIGNAL(canceled()),this,SLOT(onCancel()));
    dlg.exec();
}

void InstallProgressLoop::questionAvailable(const QString & question) {
    if (QMessageBox::question((QWidget *)parent(),Static::Question_Str,question,QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes) installer->sendAnswer('Y');
    else installer->sendAnswer('N');
}

void InstallProgressLoop::start_download(const QString & url) {
    disconnect(installer,SIGNAL(start_download(const QString &)),this,SLOT(start_download(const QString &)));
    if (wasCanceled) return;
    emit showingFilesDownloadDlg();
    disconnect(installer,SIGNAL(all_downloads_completed()),this,SLOT(all_downloads_completed()));
    FilesDownloadDialog download_dlg(installer,(QWidget *)parent());
    connect(installer,SIGNAL(all_downloads_completed()),this,SLOT(all_downloads_completed()),Qt::QueuedConnection);
    connect(&download_dlg,SIGNAL(canceled()),this,SLOT(onCancel()));
    download_dlg.setNewDownload(url);
    download_dlg.exec();
}

void InstallProgressLoop::someProvidersAvailable(const QStringList & providers) {
    emit showingProvidersList();
    QString provider = Static::someProvidersAvailable((QWidget *)parent(),providers);
    if (!provider.isEmpty()) {
        installer->sendChosenProvider(provider);
    }
    else {
        wasCanceled = true;
        installer->terminate();
    }
    emit hidingProvidersList();
}

void InstallProgressLoop::readyToProcess(double total_installed,double total_removed) {
    emit showingPackageListDlg();
    if (PackageChangesDialog(installer->install_packages(),installer->remove_packages(),total_installed,total_removed,(QWidget *)parent()).exec() == QDialog::Rejected) {
        wasCanceled = true;
        installer->cancelInstall();
        emit hidingPackageListDlg();
        return;
    }
    emit hidingPackageListDlg();

    installer->beginInstall();
}

int InstallProgressLoop::packages_count() {
    return installer->install_packages().count() + installer->remove_packages().count();
}


void InstallProgressLoop::onCancel() {
    if (QMessageBox::warning((QWidget *)parent(),Static::Warning_Str,Static::PacmanTerminate_Str,QMessageBox::Yes,QMessageBox::No) == QMessageBox::No) return;
    wasCanceled = true;
    if (installer != NULL) installer->terminate();
}

void InstallProgressLoop::installing_packages_finished(PacmanProcessReader * reader) {
    if ((reader->exitCode() != 0) || wasCanceled) {
        reader->deleteLater();
        installer = NULL;
        this->exit(QDialog::Rejected);
        return;
    }
    reader->deleteLater();
    installer = NULL;
    this->exit(QDialog::Accepted);
}

