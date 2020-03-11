/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "alpmlockingnotifier.h"
#include "libalpm.h"
#include <QDebug>

AlpmLockingNotifier::AlpmLockingNotifier(QObject *parent) : QObject(parent) {
    if (Alpm::instance() == NULL) {
        qCritical() << "AlpmLockingNotifier" << "Alpm is not initialized!";
        return;
    }
    connect(Alpm::instance(),&Alpm::locking_changed,this,[&](const QString &,bool locked) { alpm_locking_changed(locked); });
}

void AlpmLockingNotifier::alpm_locking_changed(bool locked) {
    if (Alpm::instance()->isMethodExecuting() || locked) return;
    emit unlocked();
}
