/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef SINGLEAPPLICATIONEVENTS_H
#define SINGLEAPPLICATIONEVENTS_H

#include <QObject>
#include "sharedmemory.h"

class SingleApplicationEvents : public QObject {
    Q_OBJECT
public:
    SingleApplicationEvents(int &argc,char **argv,QObject *parent = nullptr);
    bool isApplicationStarted(const QString & appname);
    bool isOtherInstanceAlreadyStarted();

    static const QString shared_key;
    static const QByteArray shared_id;

signals:
    void secondInstanceStarted(const QStringList & parms,qint64 pid);
    void applicationStarted(const QString & appname,const QStringList & parms,qint64 pid);
    void applicationExited(const QString & appname,const QStringList & parms,qint64 pid,int rc);
    void secondInstanceIAm();

private slots:
    void shm_changed();

private:
    // it's necessary to be executed in rewritten SingleApplication::exec()
    void addExitEntry(int rc);
    void addStartEntry(int &argc,char **argv);

    SharedMemory shm_obj;
    QStringList m_args;
    qint64 pid;

    friend class SingleApplication;
};

#endif // SINGLEAPPLICATIONEVENTS_H
