/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanfileslistreader.h"

PacmanFilesListReader::PacmanFilesListReader(QObject *parent) : PacmanProcessReader(parent) {
}

QString PacmanFilesListReader::command() const {
    return QString("%1/pacman -Ql").arg(TOOLS_BIN);
}

void PacmanFilesListReader::readyReadStandardOutput() {
    QString package_name;
    while (process.canReadLine()) {
        QString line = QString::fromLocal8Bit(process.readLine()).simplified();
        int index = line.indexOf(' ');
        if (index == -1) continue;
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
        if (line.isEmpty()) continue;

        if (line.endsWith('/')) {
            if (!m_dir.isEmpty()) {
                if (!line.startsWith(m_dir)) m_files.append(m_dir);
            }
            m_dir = line;
            continue;
        }

        if (!m_dir.isEmpty()) {
            m_files.append(m_dir);
            m_dir.clear();
        }
        m_files.append(line);
    }
}

void PacmanFilesListReader::error(const QString & /*error*/) {}

void PacmanFilesListReader::onFinished(int code,QProcess::ExitStatus status) {
    if (!m_package.isEmpty()) emit files_ready(m_package,m_files);
    PacmanProcessReader::onFinished(code,status);
}
