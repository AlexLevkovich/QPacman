/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmandbrefresher.h"
#include <QFile>
#include "confsettings.h"

PacmanDBRefresher::PacmanDBRefresher(const QString & su_password,QObject *parent) : PacmanProcessReader(su_password,parent) {
    tempConf = ConfSettings::createTempConfName();
}

PacmanDBRefresher::~PacmanDBRefresher() {
    QFile::remove(tempConf);
}

QString PacmanDBRefresher::command() const {
    return use_su()?QString("%2 --config %1 -Sy").arg(tempConf).arg(PACMAN_BIN):QString("%2 %1").arg(tempConf).arg(PACMANSY_BIN);
}

void PacmanDBRefresher::start() {
    ConfSettings settings(tempConf);
    if (!settings.copyFromPacmanConf()) {
        setCode(1);
        addToErrorStreamCache(tr("Cannot copy pacman.conf to %1").arg(tempConf) + "\n");
    }
    settings.replaceXferCommand();

    PacmanProcessReader::start();
}
