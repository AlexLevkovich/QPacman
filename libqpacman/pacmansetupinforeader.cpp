/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmansetupinforeader.h"

QString PacmanSetupInfoReader::pacman_cache_dir;
QString PacmanSetupInfoReader::pacman_conf;
QString PacmanSetupInfoReader::pacman_lock_file;
QString PacmanSetupInfoReader::pacman_db_path;

static const QString afterColon(const QString & str) {
    int index = str.indexOf(':');
    if (index == -1) return QString();

    return str.mid(index+1).simplified();
}

PacmanSetupInfoReader::PacmanSetupInfoReader(QObject *parent) : PacmanProcessReader(parent) {
}

QString PacmanSetupInfoReader::command() const {
    return QString("%1 -v").arg(PACMAN_BIN);
}

bool PacmanSetupInfoReader::output(const QString & out) {
    QString line = out.simplified();
    if (line.isEmpty()) return true;
    if (line.startsWith("Conf File :")) {
        pacman_conf = afterColon(line);
    }
    else if (line.startsWith("Cache Dirs:")) {
        pacman_cache_dir = afterColon(line);
        if (pacman_cache_dir.endsWith("/")) pacman_cache_dir = pacman_cache_dir.left(pacman_cache_dir.count()-1);
    }
    else if (line.startsWith("Lock File :")) {
        pacman_lock_file = afterColon(line);
    }
    else if (line.startsWith("DB Path :")) {
        pacman_db_path = afterColon(line)+"sync";
    }
    return true;
}

void PacmanSetupInfoReader::onFinished(int code,QProcess::ExitStatus status) {
    if (pacman_conf.isEmpty() || pacman_cache_dir.isEmpty() ||
        pacman_lock_file.isEmpty() || pacman_db_path.isEmpty()) {
        setCode(1);
        addToErrorStreamCache(tr("QPacman: Cannot correctly parse the pacman's output!!!\n"));
    }
    else setCode(-code);

    PacmanProcessReader::onFinished(code,status);
}

bool PacmanSetupInfoReader::error(const QString & err) {
    if (err.startsWith("error: no operation specified")) clearErrorStreamCache();
    return true;
}
