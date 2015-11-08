/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanfileslistreader.h"
#include "pacmanserverinterface.h"

PacmanFilesListReader::PacmanFilesListReader(QObject *parent) : PacmanProcessReader(parent) {
    connect(PacmanServerInterface::instance(),SIGNAL(files_ready(const QString &,const QStringList &)),this,SIGNAL(files_ready(const QString &,const QStringList &)));
}

QByteArray PacmanFilesListReader::command() const {
    return "FILES OF PACKAGES";
}

void PacmanFilesListReader::send_parameters() {}

