/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanpackagereasonchanger.h"
#include "static.h"
#include "pacmanserverinterface.h"

PacmanPackageReasonChanger::PacmanPackageReasonChanger(const QString & package,bool doDeps,QObject *parent) : PacmanProcessReader(parent) {
    m_package = package;
    m_doDeps = doDeps;
}

QByteArray PacmanPackageReasonChanger::command() const {
    return "CHANGE REASON";
}

void PacmanPackageReasonChanger::send_parameters() {
    PacmanServerInterface::instance()->setPassword(Static::encryptedPassword);
    PacmanServerInterface::instance()->setPackages(m_package);
    PacmanServerInterface::instance()->setDependance(m_doDeps);
}


