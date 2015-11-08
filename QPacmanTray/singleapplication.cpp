/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "singleapplication.h"
#include <QWidget>
#include <QMessageBox>
#include "dbuswatcher.h"

SingleApplication::SingleApplication(int & argc, char ** argv) : QApplication(argc, argv), sharedLock("/tmp/QPacmanTray_app") {
    m_mainWidget = NULL;

    if(!sharedLock.tryLock()) {
        PacmanServerInterface::createInstance(this);
        if (!PacmanServerInterface::instance()->isValid()) {
            m_isStarted = false;
            return;
        }
        PacmanServerInterface::instance()->commandRequest("SHOW TRAY");
        m_isStarted = false;
        return;
    }

    installEventFilter(this);

    PacmanServerInterface::createInstance(this);
    if (!PacmanServerInterface::instance()->isValid()) {
        QMessageBox::critical(NULL,"Error...",QObject::tr("Cannot connect to dbus' QPacmanServer!"),QMessageBox::Ok);
        return;
    }

    connect(this,SIGNAL(aboutToQuit()),this,SLOT(aboutToQuit()));
    connect(&signalHandler,SIGNAL(sigTERM()),this,SLOT(quit()));
    connect(&signalHandler,SIGNAL(sigINT()),this,SLOT(quit()));
    signalHandler.start();

    initDbusConnections();
    connect(PacmanServerInterface::watcher(),SIGNAL(loaded()),this,SLOT(dbusLoaded()));
    connect(PacmanServerInterface::watcher(),SIGNAL(unloaded()),this,SLOT(dbusUnloaded()));
    setQuitOnLastWindowClosed(false);
    m_isStarted = true;
}

void SingleApplication::guiExited() {
    if (m_mainWidget != NULL) QMetaObject::invokeMethod(m_mainWidget,"onGuiExited",Qt::QueuedConnection);
}

void SingleApplication::guiStarted() {
    if (m_mainWidget != NULL) QMetaObject::invokeMethod(m_mainWidget,"onGuiStarted",Qt::QueuedConnection);
}

bool SingleApplication::isStarted() {
    return (m_isStarted && (PacmanServerInterface::instance() != NULL) && PacmanServerInterface::instance()->isValid());
}

void SingleApplication::showMainWindow() {
    if (m_mainWidget != NULL) m_mainWidget->setVisible(true);
}

void SingleApplication::updatesList(const QStringList & list) {
    if (m_mainWidget != NULL) QMetaObject::invokeMethod(m_mainWidget,"_areUpdates",Qt::QueuedConnection,Q_ARG(const QStringList &,list));
}

void SingleApplication::aboutToQuit() {
    if (m_mainWidget != NULL) QMetaObject::invokeMethod(m_mainWidget,"terminate",Qt::DirectConnection);
}

bool SingleApplication::eventFilter(QObject * receiver,QEvent * event) {
    /*QString receiver_class = QString::fromLatin1(receiver->metaObject()->className());
    if (receiver_class.startsWith("QDBus") && (receiver_class != "QDBusServiceWatcher")) {
        //qDebug() << receiver->metaObject()->className() << event->type();
        if (event->type() == QEvent::MetaCall) return true;
    }*/
    return QApplication::eventFilter(receiver,event);
}

void SingleApplication::dbusLoaded() {
    initDbusConnections();
    if (m_mainWidget != NULL) QMetaObject::invokeMethod(m_mainWidget,"dbusLoaded",Qt::QueuedConnection);
}

void SingleApplication::initDbusConnections() {
    connect(PacmanServerInterface::instance(),SIGNAL(show_tray_window()),this,SLOT(showMainWindow()));
    connect(PacmanServerInterface::instance(),SIGNAL(client_exited()),this,SLOT(guiExited()));
    connect(PacmanServerInterface::instance(),SIGNAL(client_started()),this,SLOT(guiStarted()));
    connect(PacmanServerInterface::instance(),SIGNAL(packages_to_update(const QStringList &)),this,SLOT(updatesList(const QStringList &)));
}

void SingleApplication::dbusUnloaded() {
    disconnect(PacmanServerInterface::instance(),SIGNAL(show_tray_window()),this,SLOT(showMainWindow()));
    disconnect(PacmanServerInterface::instance(),SIGNAL(client_exited()),this,SLOT(guiExited()));
    disconnect(PacmanServerInterface::instance(),SIGNAL(client_started()),this,SLOT(guiStarted()));
    disconnect(PacmanServerInterface::instance(),SIGNAL(packages_to_update(const QStringList &)),this,SLOT(updatesList(const QStringList &)));
    if (m_mainWidget != NULL) QMetaObject::invokeMethod(m_mainWidget,"dbusUnloaded",Qt::QueuedConnection);
}
