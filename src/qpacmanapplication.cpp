/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "qpacmanapplication.h"
#include "static.h"
#include "libalpm.h"
#include "mainwindow.h"
#include "localpackagemainwindow.h"
#include "dbrefresher.h"
#include "packageinstaller.h"
#include <QFileInfo>
#include <QDir>
#include <QPlainTextEdit>
#include <QDebug>

QPacmanApplication::QPacmanApplication(int &argc, char **argv) : SingleApplication(argc,argv) {
    m_mainWindow = NULL;
    m_localPkgWindow = NULL;
    updateWindow = NULL;
    m_wasTopMost = false;
    progressView = NULL;
    cancelAction = NULL;

    connect(this,SIGNAL(firstInstanceAttempt()),this,SLOT(firstInstanceAttempted()));
    connect(this,SIGNAL(secondInstanceAttempt(const QStringList &)),this,SLOT(secondInstanceAttempted(const QStringList &)));
}

QPacmanApplication::~QPacmanApplication() {
    if (m_mainWindow != NULL) delete m_mainWindow;
    if (updateWindow != NULL) delete updateWindow;
    if (m_localPkgWindow != NULL) delete m_localPkgWindow;
}

void QPacmanApplication::firstInstanceAttempted() {
    QStringList packages;
    QStringList argv = arguments();
    if (argv.count() >= 2) {
        for (int i=1;i<argv.count();i++) {
            if (QFileInfo(argv[i]).exists()) packages.append(argv[i]);
        }
    }

    if (packages.count() > 0) {
       m_localPkgWindow = new LocalPackageMainWindow(packages);
       m_localPkgWindow->show();
    }
    else if ((argv.count() >= 2) && (QString(argv[1]) == "--update")) {
        upgradePackages();
        return;
    }
    else {
        initMainWindow();
    }
}

void QPacmanApplication::upgradePackages() {
    QString userSyncDir = qgetenv("QPACMAN_SYNC_DIR");
    if (Alpm::isOpen() && !userSyncDir.isEmpty() && Static::isLeftDirNewer(QDir(userSyncDir),QDir(Alpm::instance()->dbDirPath()+QDir::separator()+QString::fromLatin1("sync")))) {
        updateWindow = PackageProcessor::createMainProcessorWindow(&progressView,&logView,&cancelAction,&logAction);
        DBRefresher * dbr = new DBRefresher(progressView,cancelAction);
        connect(dbr,SIGNAL(completed(ThreadRun::RC)),SLOT(dbRefreshCompleted(ThreadRun::RC)),Qt::QueuedConnection);
        connect(dbr,SIGNAL(logString(const QString &)),logView,SLOT(appendPlainText(const QString &)),Qt::QueuedConnection);
    }
    else dbRefreshCompleted(ThreadRun::OK);
}

void QPacmanApplication::dbRefreshCompleted(ThreadRun::RC rc) {
    if (rc != ThreadRun::OK) {
        upgradeCompleted(rc);
        return;
    }
    if (progressView == NULL) updateWindow = PackageProcessor::createMainProcessorWindow(&progressView,&logView,&cancelAction,&logAction);
    PackageInstaller * pkg_inst = new PackageInstaller(QList<AlpmPackage *>(),QList<AlpmPackage *>(),false,progressView,cancelAction,NULL,NULL);
    connect(pkg_inst,SIGNAL(completed(ThreadRun::RC)),SLOT(upgradeCompleted(ThreadRun::RC)),Qt::QueuedConnection);
    connect(pkg_inst,SIGNAL(logString(const QString &)),logView,SLOT(appendPlainText(const QString &)),Qt::QueuedConnection);
}

void QPacmanApplication::upgradeCompleted(ThreadRun::RC) {
     cancelAction->setEnabled(true);
     logAction->setEnabled(true);
     connect(cancelAction,SIGNAL(triggered()),updateWindow,SLOT(close()));
}

void QPacmanApplication::initMainWindow() {
    m_mainWindow = new MainWindow();
    m_mainWindow->show();
}

void QPacmanApplication::secondInstanceAttempted(const QStringList & argv) {
    QStringList packages;
    if (argv.count() >= 2) {
        for (int i=1;i<argv.count();i++) {
            if (QFileInfo(argv[i]).exists()) packages.append(argv[i]);
        }
    }

    if (packages.count() > 0) {
        if (m_localPkgWindow != NULL) return;
        m_localPkgWindow = new LocalPackageMainWindow(packages);
        m_localPkgWindow->show();
    }
    else {
        if ((argv.count() >= 2) && (QString(argv[1]) == "--update")) {
            if (m_mainWindow == NULL) upgradePackages();
            else {
                QMetaObject::invokeMethod(m_mainWindow,"on_actionFullUpdate_triggered");
                putWindowOnTop(m_mainWindow);
            }
            return;
        }
        else {
            if (updateWindow != NULL) return;
            if (m_mainWindow != NULL) {
                putWindowOnTop(m_mainWindow);
            }
            else initMainWindow();
        }
    }
}

void QPacmanApplication::putWindowOnTop(QMainWindow * wnd) {
    wnd->setWindowFlags(wnd->windowFlags() | Qt::WindowStaysOnTopHint);
    m_wasTopMost = true;
    wnd->setVisible(true);
    wnd->activateWindow();
    wnd->raise();
}

bool QPacmanApplication::notify(QObject *receiver, QEvent *event) {
    bool ret = SingleApplication::notify(receiver,event);
    if (m_wasTopMost && (receiver != NULL) && (event->type() == QEvent::ActivationChange) && qobject_cast<QMainWindow *>(receiver) && ((QMainWindow *)receiver)->isActiveWindow()) {
        QMainWindow * mainWindow = (QMainWindow *)receiver;
        mainWindow->setWindowFlags(mainWindow->windowFlags() & ~Qt::WindowStaysOnTopHint);
        mainWindow->setVisible(true);
        m_wasTopMost = false;
    }
    if ((event->type() == QEvent::Close) && (receiver == m_mainWindow)) {
        connect(m_mainWindow,SIGNAL(destroyed()),this,SLOT(mainWindowDestroyed()));
        m_mainWindow->deleteLater();
    }
    if ((event->type() == QEvent::Close) && (receiver == m_localPkgWindow)) {
        connect(m_localPkgWindow,SIGNAL(destroyed()),this,SLOT(localMainWindowDestroyed()));
        m_localPkgWindow->deleteLater();
    }
    if ((event->type() == QEvent::Close) && (receiver == updateWindow)) {
        connect(updateWindow,SIGNAL(destroyed()),this,SLOT(updateWindowDestroyed()));
        updateWindow->deleteLater();
    }
    return ret;
}

void QPacmanApplication::updateWindowDestroyed() {
    updateWindow = NULL;
}

void QPacmanApplication::mainWindowDestroyed() {
    m_mainWindow = NULL;
}

void QPacmanApplication::localMainWindowDestroyed() {
    m_localPkgWindow = NULL;
}

QMainWindow * QPacmanApplication::findMainWindow() {
    QList<QMainWindow *> list = findMainWindows();
    if (list.count() <= 0) return NULL;

    QMainWindow * firstFoundWnd = NULL;
    QMainWindow * widget = NULL;
    for (int i=0;i<list.count();i++) {
        widget = list.at(i);
        if (firstFoundWnd == NULL) firstFoundWnd = widget;
        if (!strcmp(widget->metaObject()->className(),"MainWindow")) return widget;
    }

    return firstFoundWnd;
}
