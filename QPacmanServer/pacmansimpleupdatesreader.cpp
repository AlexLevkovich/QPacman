/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmansimpleupdatesreader.h"

PacmanSimpleUpdatesReader::PacmanSimpleUpdatesReader(QObject *parent) : PacmanProcessReader(parent) {
}

QString PacmanSimpleUpdatesReader::command() const {
    return QString("%1 -Qqu").arg(PACMAN_BIN);
}

bool PacmanSimpleUpdatesReader::output(const QString & out) {
    QString line = out.simplified();
    if (line.isEmpty()) return true;
    m_packages.append(line);
    return true;
}

