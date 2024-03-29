/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "updatechecker.h"
#include "dbrefresher.h"
#include "libalpm.h"
#include "alpmpackage.h"
#include "static.h"
#include <QNetworkInterface>

NetworkConfigurationChecker::NetworkConfigurationChecker(QObject * parent) : QObject(parent) {
    m_is_online = status();
    m_timer.setInterval(1000);
    connect(&m_timer,&QTimer::timeout,this,&NetworkConfigurationChecker::process);
}

void NetworkConfigurationChecker::start() {
    m_is_online = status();
    QMetaObject::invokeMethod(this,"onlineStateChanged",Qt::QueuedConnection,Q_ARG(bool,m_is_online));
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
    for (QNetworkInterface & ni: QNetworkInterface::allInterfaces()) {
        if (ni.flags().testFlag(QNetworkInterface::IsLoopBack)) continue;
        if (ni.flags().testFlag(QNetworkInterface::IsRunning)) return true;
    }
    return false;
}

UpdateChecker::UpdateChecker(QObject * parent) : PackageProcessorBase(parent) {
    connect(&network_checker,&NetworkConfigurationChecker::onlineStateChanged,this,[&](bool online) {
        if (!online) return;
        if (Alpm::instance()->executingMethodName().isEmpty()) {
            if (isQPacmanStarted()) {
                connect(Alpm::instance(),SIGNAL(method_finished(QString,ThreadRun::RC)),this,SLOT(onupdate_method_finished(QString,ThreadRun::RC)));
                Alpm::instance()->dbRefresherIsAboutToStart();
            }
            else connect(new DBRefresher(),&DBRefresher::completed,this,&UpdateChecker::oncompleted);
        }
        else QMetaObject::invokeMethod(this,"oncompleted",Qt::QueuedConnection,Q_ARG(ThreadRun::RC,ThreadRun::FORBIDDEN),Q_ARG(QString,QString()));
    });
    connect(this,&UpdateChecker::completed,this,&QObject::deleteLater,Qt::QueuedConnection);
    network_checker.start();
}

void UpdateChecker::onupdate_method_finished(const QString &,ThreadRun::RC rc) {
    oncompleted(rc,Alpm::instance()->lastError());
}

void UpdateChecker::oncompleted(ThreadRun::RC ok,const QString & error) {
    if (ok !=ThreadRun::OK) {
        emit completed((ok == ThreadRun::FORBIDDEN)?ThreadRun::OK:ok,
                       (ok == ThreadRun::FORBIDDEN)?QString():error,QStringList());
        return;
    }
    emit completed(ok,QString(),[=]() {
        QStringList ret;
        for (AlpmPackage & pkg: Alpm::instance()->updates()) {
            ret.append(pkg.name()+"-"+pkg.version());
        }
        return ret;
    }());
}
