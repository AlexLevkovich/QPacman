/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include <alpmfuture.h>
#include <QCoreApplication>
#include <QDebug>
#ifdef USE_QDBUS
#include <QDBusMetaType>
#endif
#include "libalpm.h"
#include <alpm.h>

bool ThreadRun::m_terminate = false;
QObject * ThreadRun::m_instance = NULL;
QString ThreadRun::m_method_name;

#ifdef USE_QDBUS
QDBusArgument & operator<<(QDBusArgument &argument,const ThreadRun::RC & rc) {
    argument.beginStructure();
    argument << (int)rc;
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument,ThreadRun::RC & rc) {
    argument.beginStructure();
    int val;
    argument >> val;
    rc = (ThreadRun::RC)val;
    argument.endStructure();

    return argument;
}
#endif

ThreadRun::ThreadRun(QObject * parent) : QObject(parent) {
    qRegisterMetaType<ThreadRun::RC>("ThreadRun::RC");
#ifdef USE_QDBUS
    qDBusRegisterMetaType<ThreadRun::RC>();
#endif
}

void ThreadRun::setTerminateFlag() {
    if (!isMethodExecuting()) return;
    m_terminate = true;
    alpm_trans_interrupt(Alpm::instance()->m_alpm_handle);
}
