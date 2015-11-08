/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmansetupinforeader.h"
#include "pacmanentry.h"

extern QString pacman_cache_dir;
extern QString pacman_conf;
extern QString pacman_lock_file;
extern QString pacman_db_path;

PacmanSetupInfoReader::PacmanSetupInfoReader(QObject *parent) : PacmanProcessReader(parent) {
}

QString PacmanSetupInfoReader::command() const {
    return QString("%1/pacman -v").arg(TOOLS_BIN);
}

void PacmanSetupInfoReader::readyReadStandardOutput() {
    while (process.canReadLine()) {
        QString line = QString::fromLocal8Bit(process.readLine());
        if (line.endsWith("\n")) line = line.left(line.length()-1);
        line = line.simplified();
        if (line.isEmpty()) continue;
        if (line.startsWith("Conf File :")) {
            pacman_conf = PacmanEntry::afterColon(line);
        }
        else if (line.startsWith("Cache Dirs:")) {
            pacman_cache_dir = PacmanEntry::afterColon(line);
            if (pacman_cache_dir.endsWith("/")) pacman_cache_dir = pacman_cache_dir.left(pacman_cache_dir.count()-1);
        }
        else if (line.startsWith("Lock File :")) {
            pacman_lock_file = PacmanEntry::afterColon(line);
        }
        else if (line.startsWith("DB Path :")) {
            pacman_db_path = PacmanEntry::afterColon(line)+"sync";
        }
    }
}

void PacmanSetupInfoReader::onFinished(int code,QProcess::ExitStatus status) {
    if (pacman_conf.isEmpty() || pacman_cache_dir.isEmpty() ||
        pacman_lock_file.isEmpty() || pacman_db_path.isEmpty()) {
        this->code = 1;
        m_errorStream += tr("QPacman: Cannot correctly parse the pacman's output!!!\n");
    }
    else {
        this->code = 0;
        this->code = -exitCode();
    }

    PacmanProcessReader::onFinished(code,status);
}

void PacmanSetupInfoReader::readyReadStandardError() {
    QString errorStr = QString::fromLocal8Bit(process.readAllStandardError());
    m_errorStream += errorStr;
    if (m_errorStream.startsWith("error: no operation specified")) {
        code = -1;
        m_errorStream.clear();
    }
    error(errorStr);
}
