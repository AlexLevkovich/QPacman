/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "qpacmantrayapplication.h"
#include "static.h"
#include "traypreferences.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>

QPacmanTrayApplication::QPacmanTrayApplication(int &argc, char **argv) : SingleApplication(argc,argv) {
    m_mainWindow = NULL;
    m_wasTopMost = false;

    QApplication::setQuitOnLastWindowClosed(false);

    if (isApplicationStarted("qpacman")) QMetaObject::invokeMethod(this,"qpacmanStarted",Qt::QueuedConnection);

    connect(this,SIGNAL(firstInstanceAttempt()),this,SLOT(firstInstanceAttempted()));
    connect(this,SIGNAL(secondInstanceAttempt(const QStringList &)),this,SLOT(secondInstanceAttempted(const QStringList &)));
    connect(this,SIGNAL(otherApplicationStarted(const QString &,const QStringList &,qint64)),this,SLOT(otherApplicationStarted(const QString &,const QStringList &,qint64)));
    connect(this,SIGNAL(otherApplicationExited(const QString &,const QStringList &,qint64,qint64)),this,SLOT(otherApplicationExited(const QString &,const QStringList &,qint64,qint64)));
}

QPacmanTrayApplication::~QPacmanTrayApplication() {
    if (m_mainWindow != NULL) delete m_mainWindow;
}

void QPacmanTrayApplication::firstInstanceAttempted() {
    if (arguments().contains("-session")) {
        QCoreApplication::exit(127);
        return;
    }
    int index = arguments().indexOf(QRegularExpression("--startchecktimeout=.+"));
    if (index >= 0) index = arguments().at(index).split("=").at(1).toInt();
    else index = 0;
    initMainWindow(index);
}

void QPacmanTrayApplication::initMainWindow(int timeout) {
    m_mainWindow = new TrayPreferences(timeout);
    connect(m_mainWindow,SIGNAL(showRequest()),this,SLOT(putMainWindowOnTop()));
}

void QPacmanTrayApplication::secondInstanceAttempted(const QStringList &) {
    if (m_mainWindow == NULL) initMainWindow();
    putWindowOnTop(m_mainWindow);
}

void QPacmanTrayApplication::putWindowOnTop(QMainWindow * wnd) {
    if (wnd == NULL) return;
    wnd->setWindowFlags(wnd->windowFlags() | Qt::WindowStaysOnTopHint);
    m_wasTopMost = true;
    wnd->setVisible(true);
    wnd->activateWindow();
    wnd->raise();
}

bool QPacmanTrayApplication::notify(QObject *receiver, QEvent *event) {
    bool ret = SingleApplication::notify(receiver,event);
    if (m_wasTopMost && (receiver != NULL) && (event->type() == QEvent::ActivationChange) && qobject_cast<QMainWindow *>(receiver) && ((QMainWindow *)receiver)->isActiveWindow()) {
        QMainWindow * mainWindow = (QMainWindow *)receiver;
        mainWindow->setWindowFlags(mainWindow->windowFlags() & ~Qt::WindowStaysOnTopHint);
        mainWindow->setVisible(true);
        m_wasTopMost = false;
    }
    return ret;
}

void QPacmanTrayApplication::putMainWindowOnTop() {
    secondInstanceAttempted(QStringList());
}

void QPacmanTrayApplication::otherApplicationStarted(const QString & appname,const QStringList & parms,qint64) {
    if (appname != "qpacman") return;
    emit qpacmanStarted(parms);
}

void QPacmanTrayApplication::otherApplicationExited(const QString & appname,const QStringList & parms,qint64,qint64 rc) {
    if (appname != "qpacman") return;
    emit qpacmanEnded(parms,rc);
}
