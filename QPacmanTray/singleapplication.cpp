/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "singleapplication.h"
#include <QWidget>
#include <QMessageBox>
#include <QDir>
#include <QFile>

const QString SingleApplication::client_lock_file = QDir::tempPath() + QDir::separator() + "QPacman_client";
const QString SingleApplication::tray_show_file = QDir::tempPath() + QDir::separator() + "QPacmanTray_show";

SingleApplication::SingleApplication(int & argc, char ** argv) : QApplication(argc, argv), sharedLock(QString("%1/QPacmanTray_app").arg(QDir::tempPath())) {
    m_mainWidget = NULL;
    client_loaded = false;
    m_error = false;

    if(!sharedLock.tryLock()) {
        QFile(tray_show_file).open(QIODevice::WriteOnly);
        m_error = true;
        return;
    }

    connect(&watcher,SIGNAL(directoryChanged(const QString &)),this,SLOT(tempDirectoryChanged()));
    watcher.addPath(QDir::tempPath());

    connect(&signalHandler,SIGNAL(sigTERM()),this,SLOT(terminate()));
    connect(&signalHandler,SIGNAL(sigINT()),this,SLOT(terminate()));
    signalHandler.start();

    setQuitOnLastWindowClosed(false);
}

void SingleApplication::tempDirectoryChanged() {
    if (QFile(tray_show_file).exists()) {
        QFile(tray_show_file).remove();
        if (m_mainWidget != NULL) QMetaObject::invokeMethod(m_mainWidget,"show",Qt::QueuedConnection);
    }

    if (QFile(client_lock_file).exists() && !client_loaded) {
        client_loaded = true;
        if (m_mainWidget != NULL) QMetaObject::invokeMethod(m_mainWidget,"onGuiStarted",Qt::QueuedConnection);
    }
    else if (!QFile(client_lock_file).exists() && client_loaded) {
        client_loaded = false;
        if (m_mainWidget != NULL) QMetaObject::invokeMethod(m_mainWidget,"onGuiExited",Qt::QueuedConnection);
    }
}

void SingleApplication::setMainWidget(QWidget * mainWidget) {
    m_mainWidget = mainWidget;
}

void SingleApplication::terminate() {
    if (m_mainWidget != NULL) QMetaObject::invokeMethod(m_mainWidget,"terminate");
    exit(1);
}
