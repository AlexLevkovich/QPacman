/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmaninstallpackagesreader.h"
#include "static.h"
#include "pacmanserverinterface.h"

PacmanInstallPackagesReader::PacmanInstallPackagesReader(const QString & packages,QObject *parent) : PacmanProcessReader(parent) {
    m_packages = packages;
    connect(PacmanServerInterface::instance(),SIGNAL(some_providers_available(const QStringList &)),this,SIGNAL(some_providers_available(const QStringList &)));
    connect(PacmanServerInterface::instance(),SIGNAL(question_available(const QString &)),this,SIGNAL(question_available(const QString &)));
    connect(PacmanServerInterface::instance(),SIGNAL(ready_to_process_install(const QStringList &,const QStringList &,const QStringList &,double,double)),this,SLOT(on_readyToProcess(const QStringList &,const QStringList &,const QStringList &,double,double)));
    connect(PacmanServerInterface::instance(),SIGNAL(post_messages(const QString &,const QStringList &)),this,SIGNAL(post_messages(const QString &,const QStringList &)));
    connect(PacmanServerInterface::instance(),SIGNAL(start_installing(const QString &)),this,SIGNAL(start_installing(const QString &)));
    connect(PacmanServerInterface::instance(),SIGNAL(start_removing(const QString &)),this,SIGNAL(start_removing(const QString &)));
    connect(PacmanServerInterface::instance(),SIGNAL(start_download(const QString &)),this,SIGNAL(start_download(const QString &)));
    connect(PacmanServerInterface::instance(),SIGNAL(download_progress(int)),this,SIGNAL(download_progress(int)));
    connect(PacmanServerInterface::instance(),SIGNAL(contents_length_found(int)),this,SIGNAL(contents_length_found(int)));
    connect(PacmanServerInterface::instance(),SIGNAL(all_downloads_completed()),this,SIGNAL(all_downloads_completed()));
}

QByteArray PacmanInstallPackagesReader::command() const {
    return "READ INSTALL PACKAGES";
}

void PacmanInstallPackagesReader::beginInstall() {
    PacmanServerInterface::instance()->beginInstall();
}

void PacmanInstallPackagesReader::cancelInstall() {
    PacmanServerInterface::instance()->cancelInstall();
}

QStringList PacmanInstallPackagesReader::install_packages() const {
    return m_install_packages;
}

QStringList PacmanInstallPackagesReader::remove_packages() const {
    return m_remove_packages;
}

void PacmanInstallPackagesReader::send_parameters() {
    PacmanServerInterface::instance()->setPassword(Static::encryptedPassword);
    PacmanServerInterface::instance()->setPackages(m_packages);
}

void PacmanInstallPackagesReader::on_readyToProcess(const QStringList & install_packages,const QStringList & remove_packages,const QStringList & /*local_install_packages*/,double total_installed, double total_removed) {
    m_install_packages = install_packages;
    m_remove_packages = remove_packages;
    emit ready_to_process(total_installed,total_removed);
}

void PacmanInstallPackagesReader::sendChosenProvider(const QString & provider) {
    PacmanServerInterface::instance()->setSelectedProvider(provider);
}

void PacmanInstallPackagesReader::sendAnswer(char answer) {
    PacmanServerInterface::instance()->sendAnswer((int)answer);
}
