/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "installfilesprogressloop.h"
#include "pacmaninstalllocalpackagesreader.h"

InstallFilesProgressLoop::InstallFilesProgressLoop(const QString & su_password,const QStringList & packages,QWidget *parent) : InstallProgressLoop(parent) {
    installer = new PacmanInstallLocalPackagesReader(su_password,packages,this);
    init();
}

int InstallFilesProgressLoop::packages_count() {
    return InstallProgressLoop::packages_count() + ((PacmanInstallLocalPackagesReader*)installer)->local_install_packages().count();
}
