#include "updater.h"
#include "libalpm.h"
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QAction>
#include "progressview.h"
#include "packageinstaller.h"

Updater::Updater(QObject *parent) : QObject(parent) {
    connect(&m_timer,&QTimer::timeout,this,[&]() {
        m_timer.stop();
        QObject::disconnect(conn1);
        QObject::disconnect(conn2);

        updateWindow = PackageProcessor::createMainProcessorWindow(&progressView,&logView,&cancelAction,&logAction);
        PackageInstaller * pkg_inst = new PackageInstaller(QList<AlpmPackage>(),QList<AlpmPackage>(),false,progressView,cancelAction,NULL,NULL);
        connect(pkg_inst,&PackageInstaller::completed,this,&Updater::onInstallerCompleted);
        connect((QObject *)pkg_inst,SIGNAL(logString(const QString &)),(QObject *)logView,SLOT(appendPlainText(const QString &)));
        connect(updateWindow,&QObject::destroyed,this,&Updater::deleteLater);
    });
    conn1 = connect(Alpm::instance(),&Alpm::package_updater_started,&m_timer,&QTimer::stop);
    conn2 = connect(Alpm::instance(),SIGNAL(method_finished(const QString &,ThreadRun::RC)),this,SLOT(onupdate_method_finished(const QString &,ThreadRun::RC)));
    if (Alpm::instance()->executingMethodName().isEmpty()) {
        Alpm::instance()->updaterAboutToStart();
        m_timer.start(5000);
    }
    else QMetaObject::invokeMethod(this,"onupdate_method_finished",Qt::QueuedConnection,Q_ARG(ThreadRun::RC,ThreadRun::FORBIDDEN));
}

void Updater::onInstallerCompleted(ThreadRun::RC rc) {
    cancelAction->setText(tr("Quit"));
    logAction->setEnabled(true);
    cancelAction->setEnabled(false);

    connect(sender(),&QObject::destroyed,[&]() { cancelAction->setEnabled(true); });
    connect(cancelAction,SIGNAL(triggered()),updateWindow,SLOT(close()));
    cancelAction->setEnabled(true);

    emit completed(rc);
}

void Updater::onupdate_method_finished(const QString &,ThreadRun::RC rc) {
    if (rc == ThreadRun::ROOTPW) return;
    emit completed(rc);
    deleteLater();
}
