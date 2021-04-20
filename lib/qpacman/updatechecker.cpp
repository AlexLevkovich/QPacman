#include "updatechecker.h"
#include "dbrefresher.h"
#include <QNetworkInterface>

NetworkConfigurationChecker::NetworkConfigurationChecker(QObject * parent) : QObject(parent) {
    m_is_online = status();
    m_timer.setInterval(1000);
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(process()));
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
    connect(&network_checker,&NetworkConfigurationChecker::onlineStateChanged,[&](bool online) {
        if (!online) return;
        connect(new DBRefresher(),&DBRefresher::completed,this,&UpdateChecker::oncompleted);
    });
    connect(this,&UpdateChecker::completed,this,&QObject::deleteLater,Qt::QueuedConnection);
    network_checker.start();
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
