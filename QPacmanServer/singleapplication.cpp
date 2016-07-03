/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "singleapplication.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include "pacmandbusserver.h"
#include "pacmansetupinforeader.h"

extern QString pacman_lock_file;

SingleApplication::SingleApplication(int & argc, char ** argv) : QCoreApplication(argc, argv), sharedLock(QString("%1/QPacman_server_instance").arg(QDir::tempPath())) {
    p_server = NULL;

    connect(this,SIGNAL(aboutToQuit()),this,SLOT(aboutToQuit()));
    connect(&signalHandler,SIGNAL(sigTERM()),this,SLOT(quit()));
    connect(&signalHandler,SIGNAL(sigINT()),this,SLOT(quit()));

    signalHandler.start();
    if(!sharedLock.tryLock()) {
        qCritical() << tr("One instance is already running!!!");
        ::exit(1);
        return;
    }

    PacmanSetupInfoReader inforeader;
    inforeader.waitToComplete();
    if (inforeader.exitCode() > 0) {
        qCritical() << inforeader.errorStream();
        ::exit(2);
        return;
    }

    QMetaObject::invokeMethod(this,"_start",Qt::QueuedConnection);
}

void SingleApplication::_start() {
    p_server = new PacmanDBusServer(this);
}

void SingleApplication::aboutToQuit() {
    if (!pacman_lock_file.isEmpty()) QFile::remove(pacman_lock_file);
}

void SingleApplication::quit() {
    if (p_server != NULL) p_server->terminateRequest();
    QCoreApplication::quit();
}

