/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmancachecleaner.h"

PacmanCacheCleaner::PacmanCacheCleaner(QObject *parent) : PacmanProcessReader(parent) {}

QString PacmanCacheCleaner::command() const {
    return QString(PACMANRMCACHE_BIN);
}
