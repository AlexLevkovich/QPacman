/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "suchecker.h"

SuChecker::SuChecker(const QString & password,QObject *parent) : PacmanProcessReader(parent) {
    m_ok = -1;
    m_password = password;
}

QString SuChecker::command() const {
    return QString("%1 -c true").arg(SU_BIN);
}

bool SuChecker::error(const QString & out) {
    QString line = out.simplified();
    if (line.isEmpty()) return true;
    if (line.simplified() == "Password:") {
        write(m_password.toLocal8Bit()+'\n');
    }
    else if (line == "su: Authentication failure") {
        m_ok = 0;
    }
    return true;
}

void SuChecker::onFinished(int code,QProcess::ExitStatus status) {
    setCode(-exitCode());
    PacmanProcessReader::onFinished(code,status);
}
