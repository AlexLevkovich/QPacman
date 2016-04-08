/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -v -c PacmanServerInterface -p pacmanserverinterface.h:pacmanserverinterface.cpp org.alexl.PacmanServer.xml
 *
 * qdbusxml2cpp is Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "pacmanserverinterface.h"
#include <QDBusConnection>

/*
 * Implementation of interface class PacmanServerInterface
 */

PacmanServerInterface * PacmanServerInterface::p_dbus_interface = NULL;
DBusWatcher * PacmanServerInterface::p_dbus_wacher = NULL;
QObject * PacmanServerInterface::save_parent = NULL;


PacmanServerInterface * PacmanServerInterface::createInstance(QObject *parent) {
    save_parent = parent;
    p_dbus_interface = new PacmanServerInterface(parent);
    if (p_dbus_wacher == NULL) p_dbus_wacher = new DBusWatcher(parent);
    QObject::connect(p_dbus_wacher,SIGNAL(loaded()),p_dbus_interface,SLOT(on_dbus_loaded()));
    QObject::connect(p_dbus_wacher,SIGNAL(unloaded()),p_dbus_interface,SLOT(on_dbus_unloaded()));

    return p_dbus_interface;
}

PacmanServerInterface * PacmanServerInterface::instance() {
    return p_dbus_interface;
}

DBusWatcher * PacmanServerInterface::watcher() {
    return p_dbus_wacher;
}

PacmanServerInterface::PacmanServerInterface(QObject *parent)
    : QDBusAbstractInterface(name(),path(),name(),QDBusConnection::systemBus(),parent) {
#ifdef PACMANENTRY
    connection().connect(name(),path(),name(),"package_ready",this,SLOT(on_package_ready(const QByteArray &)));
#endif
    connection().connect(name(),path(),name(),"links_ready",this,SLOT(on_links_ready(const QByteArray &)));

    QMetaObject::invokeMethod(this,"_start",Qt::QueuedConnection);
}

PacmanServerInterface::~PacmanServerInterface() {
#ifdef PACMANENTRY
    connection().disconnect(name(),path(),name(),"package_ready",this,SLOT(on_package_ready(const QByteArray &)));
#endif
    connection().disconnect(name(),path(),name(),"links_ready",this,SLOT(on_links_ready(const QByteArray &)));
}

void PacmanServerInterface::_start() {
    QDBusError err = lastError();
    if (err.isValid()) emit dbus_error(err.message());
    else if (!isValid()) emit dbus_error(tr("Invalid DBUS server interface!"));
}

#ifdef PACMANENTRY
void PacmanServerInterface::on_package_ready(const QByteArray & arr) {
    PacmanEntry entry;
    QDataStream stream(arr);
    stream >> entry;
    emit package_ready(entry);
}
#endif

void PacmanServerInterface::on_links_ready(const QByteArray & arr) {
    QList<QUrl> list;
    QDataStream stream(arr);
    stream >> list;
    emit links_ready(list);
}

void PacmanServerInterface::on_dbus_loaded() {
    disconnect(p_dbus_wacher,SIGNAL(loaded()),p_dbus_interface,SLOT(on_dbus_loaded()));
    delete p_dbus_interface;
    p_dbus_interface = NULL;
    createInstance(save_parent);
}

void PacmanServerInterface::on_dbus_unloaded() {
    disconnect(p_dbus_wacher,SIGNAL(unloaded()),p_dbus_interface,SLOT(on_dbus_unloaded()));
}

