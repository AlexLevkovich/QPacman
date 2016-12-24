/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanfileslistreader.h"

PacmanFilesListReader::PacmanFilesListReader(QObject *parent) : PacmanProcessReader(parent) {
}

QString PacmanFilesListReader::command() const {
    return QString("%1 -Ql").arg(PACMAN_BIN);
}

bool PacmanFilesListReader::output(const QString & out) {
    QString package_name;
    QString line = out.simplified();
    int index = line.indexOf(' ');
    if (index == -1) return true;
    package_name = line.left(index);
    if (package_name != m_package) {
        if (!m_package.isEmpty()) {
            if (!m_dir.isEmpty()) {
                 m_files.append(m_dir);
                 m_dir.clear();
             }
             emit files_ready(m_package,m_files);
        }
        m_package = package_name;
        m_files.clear();
    }
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
    if (!m_package.isEmpty()) emit files_ready(m_package,m_files);
    PacmanProcessReader::onFinished(code,status);
}
