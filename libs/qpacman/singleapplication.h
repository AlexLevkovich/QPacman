/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include "singleapplicationevents.h"

class QMainWindow;

class SingleApplication : public QApplication {
    Q_OBJECT
public:
    SingleApplication(int &argc, char **argv);
    QList<QMainWindow *> findMainWindows();
    virtual QMainWindow * findMainWindow();
    bool isApplicationStarted(const QString & appname);
    int exec();

protected:
    bool notify(QObject *receiver,QEvent *event);

private slots:
    void applicationStarted(const QString & appname,const QStringList & parms,qint64 pid);
    void applicationExited(const QString & appname,qint64 pid,int rc);
    void exitSecondOne();

signals:
    void firstInstanceAttempt();
    void secondInstanceAttempt(const QStringList & args);
    void otherApplicationStarted(const QString & appname);
    void otherApplicationExited(const QString & appname,qint64 rc);

private:
    QList<QMainWindow *> activeMainWinds;
    SingleApplicationEvents app_events;
};

#endif // SINGLEAPPLICATION_H
