/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "usualuserupdateschecker.h"
#include "static.h"
#include "libalpm.h"
#include "alpmdb.h"
#include <QEventLoop>
#include <QSettings>

UsualUserUpdatesChecker::UsualUserUpdatesChecker(QObject * parent) : QObject(parent) {
    QMetaObject::invokeMethod(this,"process",Qt::QueuedConnection);
}

void UsualUserUpdatesChecker::process() {
    connect(this,SIGNAL(ok(const QStringList &)),this,SLOT(deleteLater()),Qt::QueuedConnection);
    connect(this,SIGNAL(error(const QString &)),this,SLOT(deleteLater()),Qt::QueuedConnection);

    if (!Alpm::isOpen()) {
        m_last_error = tr("Alpm library is not initialized!");
        emit error(m_last_error);
        return;
    }

    emit database_updating();
    if (Alpm::instance()->updateDBs() != ThreadRun::OK) {
        m_last_error = Alpm::instance()->lastError();
        emit error(m_last_error);
        return;
    }

    if (!Alpm::instance()->reopen()) {
        m_last_error = Alpm::instance()->lastError();
        emit error(m_last_error);
        return;
    }

    emit getting_updates();
    QVector<AlpmPackage *> updates = Alpm::instance()->updates();
    for (int i=0;i<updates.count();i++) {
        m_updates.append(updates.at(i)->toString());
    }
    emit ok(m_updates);
}

const QString UsualUserUpdatesChecker::result(QStringList & updates) {
    QEventLoop loop;
    UsualUserUpdatesChecker * updater = new UsualUserUpdatesChecker();
    QObject::connect(updater,SIGNAL(ok(const QStringList &)),&loop,SLOT(quit()));
    QObject::connect(updater,SIGNAL(error(const QString &)),&loop,SLOT(quit()));
    loop.exec();

    updates = updater->updates();
    return updater->lastError();
}

QString UsualUserUpdatesChecker::lastError() const {
    return m_last_error;
}

QStringList UsualUserUpdatesChecker::updates() const {
    return m_updates;
}
