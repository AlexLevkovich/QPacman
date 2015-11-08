/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanrepositoryreader.h"
#include "pacmanserverinterface.h"

PacmanRepositoryReader::PacmanRepositoryReader(QObject *parent) : PacmanProcessReader(parent) {
    connect(PacmanServerInterface::instance(),SIGNAL(package_ready(const PacmanEntry &)),this,SIGNAL(read_package(const PacmanEntry &)));
}

QByteArray PacmanRepositoryReader::command() const {
    return "LIST OF PACKAGES";
}

void PacmanRepositoryReader::send_parameters() {}

