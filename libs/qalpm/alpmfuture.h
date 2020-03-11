/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ALPMFUTURE_H
#define ALPMFUTURE_H

#include <QObject>
#include <QFuture>
#include <QEventLoop>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

class ThreadRun : public QObject {
    Q_OBJECT
private:
    template <typename T> class AlpmFuture : public QFuture<T> {
    private:
        AlpmFuture(const QFuture<T> &other) : QFuture<T>(other) {
            QObject::connect(&watcher,SIGNAL(finished()),&loop,SLOT(quit()));
            watcher.setFuture(*this);
        }

        T exec() {
            if (!QFuture<T>::isFinished()) loop.exec();
            return QFuture<T>::result();
        }

        QFutureWatcher<T> watcher;
        QEventLoop loop;

        friend class ThreadRun;
    };

    class ThreadRunExit {
    private:
        ThreadRunExit(ThreadRun * run) {
            m_run = run;
            ThreadRun::m_instance = true;
            ThreadRun::m_terminate = false;
        }
        ~ThreadRunExit() {
            ThreadRun::m_instance = false;
            emit m_run->methodFinished(m_run);
        }

        ThreadRun * m_run;
        friend class ThreadRun;
    };
public:
    enum RC {
        OK,
        BAD,
        TERMINATED,
        FORBIDDEN
    };

    ThreadRun(QObject * parent = NULL);
    ~ThreadRun();
    static void setTerminateFlag();
    static bool isTerminateFlagSet() { return m_terminate; }
    static bool isMethodExecuting() { return m_instance; }
    static const QVector<ThreadRun *> instances() { return m_instances; }
    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
    ThreadRun::RC run(T & ret,Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4,Arg5 arg5) {
        if (isMethodExecuting()) return ThreadRun::FORBIDDEN;
        ThreadRunExit _exit(this);
        ret = AlpmFuture<T>(QtConcurrent::run(obj,func,arg1,arg2,arg3,arg4,arg5)).exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:ThreadRun::OK;
    }
    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    ThreadRun::RC run(T & ret,Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4) {
        if (isMethodExecuting()) return ThreadRun::FORBIDDEN;
        ThreadRunExit _exit(this);
        ret = AlpmFuture<T>(QtConcurrent::run(obj,func,arg1,arg2,arg3,arg4)).exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:ThreadRun::OK;
    }
    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3>
    ThreadRun::RC run(T & ret,Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3) {
        if (isMethodExecuting()) return ThreadRun::FORBIDDEN;
        ThreadRunExit _exit(this);
        ret = AlpmFuture<T>(QtConcurrent::run(obj,func,arg1,arg2,arg3)).exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:ThreadRun::OK;
    }
    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2>
    ThreadRun::RC run(T & ret,Class obj,Function func,Arg1 arg1,Arg2 arg2) {
        if (isMethodExecuting()) return ThreadRun::FORBIDDEN;
        ThreadRunExit _exit(this);
        ret = AlpmFuture<T>(QtConcurrent::run(obj,func,arg1,arg2)).exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:ThreadRun::OK;
    }
    template<typename T, typename Class, typename Function, typename Arg1>
    ThreadRun::RC run(T & ret,Class obj,Function func,Arg1 arg1) {
        if (isMethodExecuting()) return ThreadRun::FORBIDDEN;
        ThreadRunExit _exit(this);
        ret = AlpmFuture<T>(QtConcurrent::run(obj,func,arg1)).exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:ThreadRun::OK;
    }
    template<typename T, typename Class, typename Function>
    ThreadRun::RC run(T & ret,Class obj,Function func) {
        if (isMethodExecuting()) return ThreadRun::FORBIDDEN;
        ThreadRunExit _exit(this);
        ret = AlpmFuture<T>(QtConcurrent::run(obj,func)).exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:ThreadRun::OK;
    }

protected:
   bool eventFilter(QObject *obj,QEvent *event);

signals:
   void methodFinished(ThreadRun * run);

private:
    static bool m_instance;
    static bool m_terminate;
    static QVector<ThreadRun *> m_instances;
};
Q_DECLARE_METATYPE(ThreadRun::RC)

#endif // ALPMFUTURE_H
