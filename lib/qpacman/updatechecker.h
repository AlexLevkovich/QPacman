/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include "packageprocessor.h"
#include <QTimer>

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

signals:
    void completed(ThreadRun::RC ok,const QString & error,const QStringList & updates);

private:
    NetworkConfigurationChecker network_checker;
};

#endif // UPDATECHECKER_H
