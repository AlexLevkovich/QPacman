/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "installfilesprogressloop.h"
#include "pacmaninstalllocalpackagesreader.h"

InstallFilesProgressLoop::InstallFilesProgressLoop(const QString & su_password,const QStringList & packages,QWidget *parent) : InstallProgressLoop(parent) {
    installer = new PacmanInstallLocalPackagesReader(su_password,packages,this);
    connect(installer,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(installing_packages_finished(PacmanProcessReader *)));
    connect(installer,SIGNAL(start_installing(const QString &)),this,SLOT(start_installing(const QString &)));
    connect(installer,SIGNAL(start_removing(const QString &)),this,SLOT(start_removing(const QString &)));
    connect(installer,SIGNAL(post_messages(const QString &,const QStringList &)),this,SIGNAL(post_messages(const QString &,const QStringList &)));
    connect(installer,SIGNAL(ready_to_process(double,double)),this,SLOT(readyToProcess(double,double)));
    connect(installer,SIGNAL(some_providers_available(const QStringList &)),this,SLOT(someProvidersAvailable(const QStringList &)));
    connect(installer,SIGNAL(question_available(const QString &)),this,SLOT(questionAvailable(const QString &)));
    connect(installer,SIGNAL(start_download(const QString &)),this,SLOT(start_download(const QString &)));
    connect(installer,SIGNAL(all_downloads_completed()),this,SLOT(all_downloads_completed()));
}

int InstallFilesProgressLoop::packages_count() {
    return InstallProgressLoop::packages_count() + ((PacmanInstallLocalPackagesReader*)installer)->local_install_packages().count();
}
