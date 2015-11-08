/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmandbrefresher.h"
#include <QMainWindow>
#include "errordialog.h"

PacmanDBRefresher::PacmanDBRefresher(QObject *parent) : PacmanProcessReader(parent) {
}

QByteArray PacmanDBRefresher::command() const {
    return "DB REFRESH";
}

void PacmanDBRefresher::send_parameters() {}

