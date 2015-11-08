/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmaninstalllocalpackagesreader.h"
#include "static.h"
#include "pacmanserverinterface.h"

PacmanInstallLocalPackagesReader::PacmanInstallLocalPackagesReader(const QStringList & packages,QObject *parent) : PacmanInstallPackagesReader("",parent) {
    m_packages = packages;
}

QByteArray PacmanInstallLocalPackagesReader::command() const {
    return "READ LOCAL INSTALL PACKAGES";
}

void PacmanInstallLocalPackagesReader::send_parameters() {
    PacmanServerInterface::instance()->setPassword(Static::encryptedPassword);
    PacmanServerInterface::instance()->setPackageList(m_packages);
}

void PacmanInstallLocalPackagesReader::on_readyToProcess(const QStringList & install_packages,const QStringList & remove_packages,const QStringList & local_install_packages,qreal total_installed, qreal total_removed) {
    m_local_install_packages = local_install_packages;
    PacmanInstallPackagesReader::on_readyToProcess(install_packages,remove_packages,local_install_packages,total_installed,total_removed);
}

QStringList PacmanInstallLocalPackagesReader::local_install_packages() const {
    return m_local_install_packages;
}
