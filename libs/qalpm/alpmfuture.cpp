/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include <alpmfuture.h>
#include <QCoreApplication>
#include <QDebug>
#include "libalpm.h"

bool ThreadRun::m_terminate = false;
bool ThreadRun::m_instance = false;
QVector<ThreadRun *> ThreadRun::m_instances;

ThreadRun::ThreadRun(QObject * parent) : QObject(parent) {
    qRegisterMetaType<ThreadRun::RC>("ThreadRun::RC");
    qApp->installEventFilter(this);
    m_instances.append(this);
}

ThreadRun::~ThreadRun() {
    m_instances.removeAll(this);
}

void ThreadRun::setTerminateFlag() {
    if (!isMethodExecuting()) return;
    m_terminate = true;
    alpm_trans_interrupt(Alpm::instance()->m_alpm_handle);
}

bool ThreadRun::eventFilter(QObject *obj,QEvent *event) {
    if ((event->type() == QEvent::Close) && obj->inherits("QWidgetWindow") && isMethodExecuting() && (obj->parent() == NULL)) return true;
    return QObject::eventFilter(obj,event);
}
