/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmancachecleaner.h"
#include "pacmansetupinforeader.h"

PacmanCacheCleaner::PacmanCacheCleaner(const QString & su_password,QObject *parent) : PacmanProcessReader(su_password,parent) {}

QString PacmanCacheCleaner::command() const {
    return QString("%2 -f %1/*").arg(PacmanSetupInfoReader::pacman_cache_dir).arg(RM_BIN);
}
