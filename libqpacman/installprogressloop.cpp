/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "installprogressloop.h"
#include "installprogressdialog.h"
#include "packagechangesdialog.h"
#include "filesdownloaddialog.h"
#include "pacmaninstallpackagesreader.h"
#include "pacmanprovidersdialog.h"
#include "static.h"
#include <QMessageBox>
#include <QKeyEvent>

#define START_WAITING_MODE if (!waiting_mode) { \
                               emit start_waiting_mode(); \
                               waiting_mode = true; \
                           }

#define END_WAITING_MODE   if (waiting_mode) { \
                               emit end_waiting_mode(); \
                               waiting_mode = false; \
                           }

const QString someProvidersAvailableFunc(QWidget * parent,const QStringList & providers) {
    PacmanProvidersDialog dlg(providers,parent);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.provider();
    }
    return QString();
}

void InstallProgressLoop::init() {
    wasCanceled = false;
    waiting_mode = true;
    connect(installer,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(installing_packages_finished(PacmanProcessReader *)));
    connect(installer,SIGNAL(post_messages(const QString &,const QStringList &)),this,SIGNAL(post_messages(const QString &,const QStringList &)));
    connect(installer,SIGNAL(ready_to_process(double,double)),this,SLOT(readyToProcess(double,double)));
    connect(installer,SIGNAL(some_providers_available(const QStringList &)),this,SLOT(someProvidersAvailable(const QStringList &)));
    connect(installer,SIGNAL(question_available(const QString &)),this,SLOT(questionAvailable(const QString &)));
    connect(installer,SIGNAL(start_download(const QString &)),this,SLOT(start_download(const QString &)));
    connect(installer,SIGNAL(all_downloads_completed()),this,SLOT(all_downloads_completed()),Qt::QueuedConnection);
    QMetaObject::invokeMethod(this,"start_waiting_mode",Qt::QueuedConnection);
}


InstallProgressLoop::InstallProgressLoop(const QString & su_password,const QStringList & packages,QWidget *parent) : QEventLoop(parent) {
    installer = new PacmanInstallPackagesReader(su_password,packages.join(" "),this);
    init();
}

InstallProgressLoop::InstallProgressLoop(const QString & su_password,QWidget *parent) : QEventLoop(parent) {
    installer = new PacmanUpdatePackagesReader(su_password,this);
    init();
}

InstallProgressLoop::InstallProgressLoop(QWidget *parent) : QEventLoop(parent) {
    wasCanceled = false;
    waiting_mode = false;
    installer = NULL;
}

void InstallProgressLoop::all_downloads_completed() {
    int total = packages_count();
    END_WAITING_MODE
    InstallProgressDialog dlg(installer,(total == 1)?0:total,(QWidget *)parent());
    connect(&dlg,SIGNAL(canceled()),this,SLOT(onCancel()));
    connect(installer,SIGNAL(finished(PacmanProcessReader *)),&dlg,SLOT(reject()));
    dlg.exec();
}

void InstallProgressLoop::questionAvailable(const QString & question) {
    END_WAITING_MODE
    if (QMessageBox::question((QWidget *)parent(),Static::Question_Str,question,QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes) installer->sendAnswer('Y');
    else installer->sendAnswer('N');
    START_WAITING_MODE
}

void InstallProgressLoop::start_download(const QString & url) {
    disconnect(installer,SIGNAL(start_download(const QString &)),this,SLOT(start_download(const QString &)));
    if (wasCanceled) return;
    disconnect(installer,SIGNAL(all_downloads_completed()),this,SLOT(all_downloads_completed()));
    END_WAITING_MODE
    FilesDownloadDialog download_dlg(installer,(QWidget *)parent());
    connect(installer,SIGNAL(all_downloads_completed()),this,SLOT(all_downloads_completed()),Qt::QueuedConnection);
    connect(&download_dlg,SIGNAL(canceled()),this,SLOT(onCancel()));
    connect(installer,SIGNAL(finished(PacmanProcessReader *)),&download_dlg,SLOT(reject()));
    download_dlg.setNewDownload(url);
    if (download_dlg.exec() == QDialog::Accepted) {
        START_WAITING_MODE
    }
}

void InstallProgressLoop::someProvidersAvailable(const QStringList & providers) {
    END_WAITING_MODE
    QString provider = someProvidersAvailableFunc((QWidget *)parent(),providers);
    if (!provider.isEmpty()) {
        installer->sendChosenProvider(provider);
        START_WAITING_MODE
    }
    else {
        wasCanceled = true;
        installer->terminate();
    }
}

void InstallProgressLoop::readyToProcess(double total_installed,double total_removed) {
    END_WAITING_MODE
    PackageChangesDialog dlg(installer->install_packages(),installer->remove_packages(),total_installed,total_removed,(QWidget *)parent());
    connect(installer,SIGNAL(finished(PacmanProcessReader *)),&dlg,SLOT(reject()));
    if (dlg.exec() == QDialog::Rejected) {
        wasCanceled = true;
        installer->cancelInstall();
        return;
    }

    START_WAITING_MODE
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
    reader->deleteLater();
    installer = NULL;
    END_WAITING_MODE
    exit(((reader->exitCode() != 0) || wasCanceled)?QDialog::Rejected:QDialog::Accepted);
}

