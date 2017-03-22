/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmaninstallpackagesreader.h"

PacmanInstallPackagesReader::PacmanInstallPackagesReader(const QString & su_password,const QString & packages,QObject *parent) : PacmanUpdatePackagesReader(su_password,parent) {
    in_packages = packages;
}

QString PacmanInstallPackagesReader::command() const {
    return QString("%3 --config %2 -S --noprogressbar %1").arg(in_packages).arg(tempConf).arg(PACMAN_BIN);
}
