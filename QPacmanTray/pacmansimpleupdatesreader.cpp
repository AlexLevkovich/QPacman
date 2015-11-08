/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmansimpleupdatesreader.h"
#include "pacmanserverinterface.h"

PacmanSimpleUpdatesReader::PacmanSimpleUpdatesReader(QObject *parent) : PacmanProcessReader(parent) {
    connect(PacmanServerInterface::instance(),SIGNAL(packages_to_update(const QStringList &)),this,SLOT(on_packages_ready(const QStringList &)));
}

QByteArray PacmanSimpleUpdatesReader::command() const {
    return "READ UPDATES";
}

void PacmanSimpleUpdatesReader::send_parameters() {}

void PacmanSimpleUpdatesReader::on_packages_ready(const QStringList & packages) {
    m_packages = packages;
}
