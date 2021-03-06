/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include <singleapplication.h>
#include <QDBusConnection>
#include <QDir>
#include "qpacmanservice.h"
#include "sigwatch.h"
#include "stacktracer.h"
#include "malloc.h"

int main(int argc, char *argv[]) {
    mallopt(M_MXFAST,0);
    mallopt(M_ARENA_MAX,2);
    mallopt(M_TRIM_THRESHOLD,131072);

    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("qpacman_service");

    SingleApplication app(argc, argv,false,SingleApplication::System);

    if (getuid() != 0) {
        qCritical() << "You must be root!!!";
        return 1;
    }

    if (!QDBusConnection::systemBus().isConnected()) {
        qCritical() << "Cannot connect to the D-Bus session bus.\n"
                       "Please check your system settings and try again";
        return 1;
    }

    StackTracer tracer(QString(LOGDIR)+QDir::separator()+"qpacman_service.log");
    QPacmanService service;

    UnixSignalWatcher sigwatch;
    sigwatch.watchForSignal(SIGINT);
    sigwatch.watchForSignal(SIGTERM);
    sigwatch.watchForSignal(SIGQUIT);
    sigwatch.watchForSignal(SIGHUP);
    QObject::connect(&sigwatch,&UnixSignalWatcher::unixSignal,&app,&SingleApplication::quit);

    return app.exec();
}
