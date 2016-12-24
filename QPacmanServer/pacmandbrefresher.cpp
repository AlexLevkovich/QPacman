/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmandbrefresher.h"
#include "pacmaninstallpackagesreader.h"
#include <QFile>
#include "confsettings.h"

extern QString pacman_conf;

PacmanDBRefresher::PacmanDBRefresher(QObject *parent) : PacmanProcessReader(parent) {
    tempConf = ConfSettings::createTempConfName();
}

PacmanDBRefresher::~PacmanDBRefresher() {
    QFile::remove(tempConf);
}

QString PacmanDBRefresher::command() const {
    return QString("%1 -i0 -o0 -e0 %3 --config %2 -Sy").arg(STDBUF_BIN).arg(tempConf).arg(PACMAN_BIN);
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
