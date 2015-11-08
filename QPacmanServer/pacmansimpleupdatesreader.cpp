/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmansimpleupdatesreader.h"

PacmanSimpleUpdatesReader::PacmanSimpleUpdatesReader(QObject *parent) : PacmanProcessReader(parent) {
}

QString PacmanSimpleUpdatesReader::command() const {
    return QString("%1/pacman -Qqu").arg(TOOLS_BIN);
}

void PacmanSimpleUpdatesReader::readyReadStandardOutput() {
    while (process.canReadLine()) {
        QString line = QString::fromLocal8Bit(process.readLine());
        if (line.endsWith("\n")) line = line.left(line.length()-1);
        line = line.simplified();
        if (line.isEmpty()) continue;
        m_packages.append(line);
    }
}

