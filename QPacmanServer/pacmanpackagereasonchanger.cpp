/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanpackagereasonchanger.h"

PacmanPackageReasonChanger::PacmanPackageReasonChanger(const QString & package,bool doDeps,QObject *parent) : PacmanProcessReader(parent) {
    m_package = package;
    m_doDeps = doDeps;
}

QString PacmanPackageReasonChanger::command() const {
    return QString("%3/pacman -D --%1 %2").arg(m_doDeps?"asdeps":"asexplicit").arg(m_package).arg(TOOLS_BIN);
}

void PacmanPackageReasonChanger::readyReadStandardOutput() {
    PacmanProcessReader::readyReadStandardOutput();
}

void PacmanPackageReasonChanger::onFinished(int code,QProcess::ExitStatus status) {
    PacmanProcessReader::onFinished(code,status);
}
