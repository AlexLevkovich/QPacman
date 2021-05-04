/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include "packageprocessor.h"
#include <QTimer>
#include <QMetaObject>

class NetworkConfigurationChecker : public QObject {
    Q_OBJECT
public:
    NetworkConfigurationChecker(QObject * parent = NULL);
    void start();
    void stop();
    bool isOnline();

private slots:
    void process();

signals:
    void onlineStateChanged(bool online);

private:
    bool status();

    QTimer m_timer;
    bool m_is_online;
};

class UpdateChecker : public PackageProcessorBase {
    Q_OBJECT
public:
    UpdateChecker(QObject * parent = NULL);

private slots:
    void oncompleted(ThreadRun::RC ok,const QString & error);
    void onupdate_method_finished(const QString & fname,ThreadRun::RC rc);

signals:
    void completed(ThreadRun::RC ok,const QString & error,const QStringList & updates);

private:
    NetworkConfigurationChecker network_checker;
    QTimer m_timer;
    QMetaObject::Connection conn1;
    QMetaObject::Connection conn2;
};

#endif // UPDATECHECKER_H
