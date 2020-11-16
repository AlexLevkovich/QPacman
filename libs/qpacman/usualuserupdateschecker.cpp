/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "usualuserupdateschecker.h"
#include "static.h"
#include "libalpm.h"
#include "alpmconfig.h"
#include "alpmdb.h"
#include <QEventLoop>
#include <QSettings>
#include <QNetworkInterface>
#include <QCoreApplication>

NetworkConfigurationChecker::NetworkConfigurationChecker(QObject * parent) : QObject(parent) {
    m_is_online = status();
    m_timer.setInterval(1000);
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(process()));
}

void NetworkConfigurationChecker::start() {
    m_is_online = status();
    m_timer.start();
}

void NetworkConfigurationChecker::stop() {
    m_timer.stop();
}

bool NetworkConfigurationChecker::isOnline() {
    return m_is_online;
}

void NetworkConfigurationChecker::process() {
    if (status() != m_is_online) {
        m_is_online = !m_is_online;
        emit onlineStateChanged(m_is_online);
    }
}

bool NetworkConfigurationChecker::status() {
    for (QNetworkInterface ni: QNetworkInterface::allInterfaces()) {
        if (ni.flags() & QNetworkInterface::IsLoopBack) continue;
        if (ni.flags() & QNetworkInterface::IsRunning) return true;
    }
    return false;
}

UsualUserUpdatesChecker::UsualUserUpdatesChecker(QObject * parent) : QObject(parent) {
    m_started = false;
    network_checker.start();

    connect(qApp,SIGNAL(aboutToQuit()),this,SLOT(aboutToQuit()));
    connect(this,SIGNAL(ok(const QStringList &)),this,SLOT(deleteLater()),Qt::QueuedConnection);
    connect(this,SIGNAL(error(const QString &,int)),this,SLOT(deleteLater()),Qt::QueuedConnection);
    connect(&network_checker,&NetworkConfigurationChecker::onlineStateChanged,[&](bool online) { if (online && !m_started) QMetaObject::invokeMethod(this,"process",Qt::QueuedConnection); });
    QMetaObject::invokeMethod(this,"process",Qt::QueuedConnection);
}

void UsualUserUpdatesChecker::aboutToQuit() {
    if (!m_started) {
        network_checker.stop();
        emit ok(QStringList());
        return;
    }
}

void UsualUserUpdatesChecker::process() {
    if (!Alpm::isOpen()) {
        m_last_error = tr("Alpm library is not initialized!");
        emit error(m_last_error,Alpm::ALPM_IS_NOT_OPEN);
        return;
    }

    if (m_started) {
        network_checker.stop();
        return;
    }
    if (!network_checker.isOnline()) return;

    network_checker.stop();
    m_started = true;

    emit database_updating();
    if ((Alpm::instance()->updateDBs() != ThreadRun::OK) || !Alpm::instance()->reopen()) {
        int err_id;
        m_last_error = Alpm::instance()->lastError(&err_id);
        emit error(m_last_error,err_id);
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
