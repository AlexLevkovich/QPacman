/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanpasswordchecker.h"
#include "pacmanserverinterface.h"

PacmanPasswordChecker::PacmanPasswordChecker(const QByteArray & password,QObject *parent) : PacmanProcessReader(parent) {
    m_password = password;
}

QByteArray PacmanPasswordChecker::command() const {
    return "PASSWORD";
}

void PacmanPasswordChecker::send_parameters() {
    PacmanServerInterface::instance()->setPassword(m_password);
}

void PacmanPasswordChecker::error(const QString & /*error*/) {}
