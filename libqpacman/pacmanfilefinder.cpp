/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanfilefinder.h"

PacmanFileFinder::PacmanFileFinder(const QString & filePath,QObject *parent) : PacmanProcessReader(parent) {
    m_filePath = filePath;
    m_searchStr = m_filePath+" is owned by ";
}

QString PacmanFileFinder::command() const {
    return QString("%1 -Qo '%2'").arg(PACMAN_BIN).arg(m_filePath);
}

bool PacmanFileFinder::output(const QString & out) {
    if (out.startsWith(m_searchStr)) {
        m_package = out.mid(m_searchStr.length());
        m_package.replace(' ','=');
    }
    return true;
}

void PacmanFileFinder::onFinished(int code,QProcess::ExitStatus status) {
    if (code > 0) {
        setCode(-code);
        clearErrorStreamCache();
    }

    PacmanProcessReader::onFinished(code,status);
}
