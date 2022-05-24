/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "dbrefresher.h"
#include <QMessageBox>
#include <QShowEvent>
#include <QPushButton>
#include <QTimer>
#include "messagedialog.h"
#include "static.h"
#include "libalpm.h"
#include <QDebug>

DBRefresher::DBRefresher(ProgressView * view,QAction * cancelAction,QObject *parent) : PackageProcessor(view,cancelAction,nullptr,parent) {}

ThreadRun::RC DBRefresher::process(const QString &) {
    return Alpm::instance()->updateDBs();
}
