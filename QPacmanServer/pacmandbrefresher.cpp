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
    return QString("%1/stdbuf -i0 -o0 -e0 %1/pacman --config %2 -Sy").arg(TOOLS_BIN).arg(tempConf);
}

void PacmanDBRefresher::start() {
    ConfSettings settings(tempConf);
    if (!settings.copyFromPacmanConf()) {
        code = 1;
        m_errorStream += tr("Cannot copy pacman.conf to %1").arg(tempConf) + "\n";
    }
    settings.replaceXferCommand();

    PacmanProcessReader::start();
}
