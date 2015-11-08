/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanfilepackageinforeader.h"
#include "pacmanserverinterface.h"

PacmanFilePackageInfoReader::PacmanFilePackageInfoReader(const QString & package,QObject *parent) : PacmanProcessReader(parent) {
    m_package = package;
    connect(PacmanServerInterface::instance(),SIGNAL(package_ready(const PacmanEntry &)),this,SLOT(on_package_ready(const PacmanEntry &)));
}

QByteArray PacmanFilePackageInfoReader::command() const {
    return "FILE INFO";
}

void PacmanFilePackageInfoReader::send_parameters() {
    PacmanServerInterface::instance()->setFilePath(m_package);
}

void PacmanFilePackageInfoReader::on_package_ready(const PacmanEntry & entry) {
    this->entry = entry;
}
