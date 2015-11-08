/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "dbuswatcher.h"
#include "pacmanserverinterface.h"

DBusWatcher::DBusWatcher(QObject *parent) : QObject(parent) {
    m_system.setConnection(QDBusConnection::systemBus());
    m_system.addWatchedService(PacmanServerInterface::name());
    m_system.setWatchMode(QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);

    connect(&m_system,SIGNAL(serviceRegistered(QString)),this,SLOT(serviceLoaded()));
    connect(&m_system,SIGNAL(serviceUnregistered(QString)),this,SLOT(serviceUnloaded()));
}

void DBusWatcher::serviceLoaded() {
    emit loaded();
}

void DBusWatcher::serviceUnloaded() {
    emit unloaded();
}
