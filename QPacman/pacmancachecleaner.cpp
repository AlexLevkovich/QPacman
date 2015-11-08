/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmancachecleaner.h"
#include "static.h"
#include "pacmanserverinterface.h"

PacmanCacheCleaner::PacmanCacheCleaner(QObject *parent) : PacmanProcessReader(parent) {
}

QByteArray PacmanCacheCleaner::command() const {
    return "CLEAN CACHE";
}

void PacmanCacheCleaner::send_parameters() {
    PacmanServerInterface::instance()->setPassword(Static::encryptedPassword);
}

