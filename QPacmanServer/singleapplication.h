/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QCoreApplication>
#include "qlockfile.h"
#include "jsondbsignals.h"

class PacmanDBusServer;

class SingleApplication : public QCoreApplication {
    Q_OBJECT
public:
    explicit SingleApplication(int & argc, char ** argv);

private slots:
    void aboutToQuit();
    void quit();
    void _start();

private:
    JsonDbSignals signalHandler;
    PacmanDBusServer * p_server;
    QLockFile sharedLock;
};

#endif // SINGLEAPPLICATION_H
