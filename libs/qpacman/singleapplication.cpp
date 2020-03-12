/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "singleapplication.h"
#include <QWidget>
#include <QFileInfo>
#include <QDebug>

SingleApplication::SingleApplication(int &argc, char **argv) : QApplication(argc,argv),
                                                               app_events(argc,argv) {
    if (applicationName().isEmpty()) setApplicationName(QFileInfo(QString::fromLocal8Bit(argv[0])).fileName());

    connect(&app_events,SIGNAL(secondInstanceStarted(const QStringList &,qint64)),SIGNAL(secondInstanceAttempt(const QStringList &)));
    connect(&app_events,SIGNAL(secondInstanceIAm()),SLOT(exitSecondOne()));
    connect(&app_events,SIGNAL(applicationStarted(const QString &,const QStringList &,qint64)),SLOT(applicationStarted(const QString &,const QStringList &,qint64)));
    connect(&app_events,SIGNAL(applicationExited(const QString &,const QStringList &,qint64,int)),SLOT(applicationExited(const QString &,const QStringList &,qint64,int)));
    if (!app_events.isOtherInstanceAlreadyStarted()) QMetaObject::invokeMethod(this,"firstInstanceAttempt",Qt::QueuedConnection);
}

void SingleApplication::exitSecondOne() {
    QCoreApplication::exit(127);
}

void SingleApplication::applicationStarted(const QString & appname,const QStringList & parms,qint64 pid) {
    emit otherApplicationStarted(QFileInfo(appname).fileName(),parms,pid);
}

void SingleApplication::applicationExited(const QString & appname,const QStringList & parms,qint64 pid,int rc) {
    emit otherApplicationExited(QFileInfo(appname).fileName(),parms,pid,rc);
}

QList<QMainWindow *> SingleApplication::findMainWindows() {
    return activeMainWinds;
}

QMainWindow * SingleApplication::findMainWindow() {
    QList<QMainWindow *> list = findMainWindows();
    if (list.count() <= 0) return NULL;

    return list.at(0);
}

bool SingleApplication::notify(QObject *receiver,QEvent *e) {
    bool ret = QApplication::notify(receiver,e);
    if ((e->type() == QEvent::Close) && receiver->inherits("QMainWindow")) {
        activeMainWinds.removeAll((QMainWindow*)receiver);
    }
    else  if ((e->type() == QEvent::Show) && receiver->inherits("QMainWindow")) {
        activeMainWinds.append((QMainWindow*)receiver);
    }
    return ret;
}

bool SingleApplication::isApplicationStarted(const QString & appname) {
    return app_events.isApplicationStarted(appname);
}

int SingleApplication::exec() {
    QString fontname = QString::fromLocal8Bit(qgetenv("ORIGINAL_FONT"));
    QFont font;
    if (!fontname.isEmpty() && font.fromString(fontname)) QApplication::setFont(font);
    int code = QApplication::exec();
    app_events.addExitEntry(code);
    return code;
}
