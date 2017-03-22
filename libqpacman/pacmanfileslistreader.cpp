/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanfileslistreader.h"

PacmanFilesListReader::PacmanFilesListReader(const QString & package,QObject *parent) : PacmanProcessReader(parent) {
    m_package = package;
}

QString PacmanFilesListReader::command() const {
    return QString("%1 -Ql %2").arg(PACMAN_BIN).arg(m_package);
}

QString PacmanFilesListReader::package() const {
    return m_package;
}

bool PacmanFilesListReader::output(const QString & out) {
    QString line = out.simplified();
    int index = line.indexOf(' ');
    if (index == -1) return true;
    line = line.mid(index+1);
    if (line.isEmpty()) return true;

    if (line.endsWith('/')) {
        if (!m_dir.isEmpty()) {
            if (!line.startsWith(m_dir)) m_files.append(m_dir);
        }
        m_dir = line;
        return true;
    }

    if (!m_dir.isEmpty()) {
         m_files.append(m_dir);
         m_dir.clear();
    }
    m_files.append(line);
    return true;
}

void PacmanFilesListReader::onFinished(int code,QProcess::ExitStatus status) {
    if (!m_package.isEmpty()) emit files_ready(m_files);
    PacmanProcessReader::onFinished(code,status);
}
