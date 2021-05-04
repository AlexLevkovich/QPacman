/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ALPMFUTURE_H
#define ALPMFUTURE_H

#include <QObject>
#include <QFuture>
#include <QEvent>
#include <QFutureWatcher>
#include <QCoreApplication>
#include <QEventLoop>
#ifdef USE_QDBUS
#include <QDBusArgument>
#endif
#include <QtConcurrent/QtConcurrent>
#include <execinfo.h>

typedef struct bfd bfd;
extern "C" char *bfd_demangle (bfd *, const char *, int);

class ThreadRun : public QObject {
    Q_OBJECT
public:
    template <typename T> class AlpmFuture : public QFuture<T>, public QObject {
    public:
        AlpmFuture(ThreadRun * parent,const QFuture<T> &other) : QFuture<T>(other), QObject(NULL) {
            ThreadRun::m_instance = this;
            ThreadRun::m_terminate = false;
            QObject::connect(&watcher,SIGNAL(finished()),&loop,SLOT(quit()));
            QObject::connect(&watcher,&QFutureWatcher<T>::finished,parent,[=](){parent->alpmFutureFinished(QFuture<T>::result());deleteLater();},Qt::QueuedConnection);
            watcher.setFuture(*this);
        }

        T exec() {
            if (!QFuture<T>::isFinished()) loop.exec();
            return QFuture<T>::result();
        }
    private:

        QFutureWatcher<T> watcher;
        QEventLoop loop;

        friend class ThreadRun;
    };

    class AlpmFutureVoid : public QFuture<void>, public QObject {
    public:
        AlpmFutureVoid(ThreadRun * parent,const QFuture<void> &other) : QFuture<void>(other), QObject(NULL) {
            ThreadRun::m_instance = this;
            ThreadRun::m_terminate = false;
            QObject::connect(&watcher,SIGNAL(finished()),&loop,SLOT(quit()));
            QObject::connect(&watcher,&QFutureWatcher<void>::finished,parent,[=](){parent->alpmFutureFinished();deleteLater();},Qt::QueuedConnection);
            watcher.setFuture(*this);
        }

        void exec() {
            if (!QFuture<void>::isFinished()) loop.exec();
        }

    private:
        QFutureWatcher<void> watcher;
        QEventLoop loop;

        friend class ThreadRun;
    };


    enum RC {
        OK = 0,
        BAD = 1,
        TERMINATED = 2,
        FORBIDDEN = 3,
        ROOTPW = 4
    };

#ifdef USE_QDBUS
    friend QDBusArgument & operator<<(QDBusArgument &argument,const ThreadRun::RC & rc);
    friend const QDBusArgument & operator>>(const QDBusArgument &argument,ThreadRun::RC & rc);
#endif

    ThreadRun(QObject * parent = NULL);
    static void setTerminateFlag();
    static bool isTerminateFlagSet() { return m_terminate; }
    static bool isMethodExecuting() { return (m_instance != NULL); }
    static const QString executingMethodName() {
        return m_method_name;
    }

    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
    ThreadRun::RC exec(T & ret,Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4,Arg5 arg5) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        ret = (new AlpmFuture<T>(this,QtConcurrent::run(obj,func,arg1,arg2,arg3,arg4,arg5)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }
    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    ThreadRun::RC exec(T & ret,Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        ret = (new AlpmFuture<T>(this,QtConcurrent::run(obj,func,arg1,arg2,arg3,arg4)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }
    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3>
    ThreadRun::RC exec(T & ret,Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        ret = (new AlpmFuture<T>(this,QtConcurrent::run(obj,func,arg1,arg2,arg3)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }
    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2>
    ThreadRun::RC exec(T & ret,Class obj,Function func,Arg1 arg1,Arg2 arg2) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        ret = (new AlpmFuture<T>(this,QtConcurrent::run(obj,func,arg1,arg2)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }
    template<typename T, typename Class, typename Function, typename Arg1>
    ThreadRun::RC exec(T & ret,Class obj,Function func,Arg1 arg1) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        ret = (new AlpmFuture<T>(this,QtConcurrent::run(obj,func,arg1)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }
    template<typename T, typename Class, typename Function>
    ThreadRun::RC exec(T & ret,Class obj,Function func) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        ret = (new AlpmFuture<T>(this,QtConcurrent::run(obj,func)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }

    template<typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
    ThreadRun::RC exec_void(Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4,Arg5 arg5) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        (new AlpmFutureVoid(this,QtConcurrent::run(obj,func,arg1,arg2,arg3,arg4,arg5)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }
    template<typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    ThreadRun::RC exec_void(Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        (new AlpmFutureVoid(this,QtConcurrent::run(obj,func,arg1,arg2,arg3,arg4)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }
    template<typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3>
    ThreadRun::RC exec_void(Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        (new AlpmFutureVoid(this,QtConcurrent::run(obj,func,arg1,arg2,arg3)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }
    template<typename Class, typename Function, typename Arg1, typename Arg2>
    ThreadRun::RC exec_void(Class obj,Function func,Arg1 arg1,Arg2 arg2) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        (new AlpmFutureVoid(this,QtConcurrent::run(obj,func,arg1,arg2)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }
    template<typename Class, typename Function, typename Arg1>
    ThreadRun::RC exec_void(Class obj,Function func,Arg1 arg1) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        (new AlpmFutureVoid(this,QtConcurrent::run(obj,func,arg1)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }
    template<typename Class, typename Function>
    ThreadRun::RC exec_void(Class obj,Function func) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        (new AlpmFutureVoid(this,QtConcurrent::run(obj,func)))->exec();
        return isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC();
    }

    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
    ThreadRun::RC run(Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4,Arg5 arg5) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        new AlpmFuture<T>(this,QtConcurrent::run(obj,func,arg1,arg2,arg3,arg4,arg5));
        return ThreadRun::OK;
    }
    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    ThreadRun::RC run(Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        new AlpmFuture<T>(this,QtConcurrent::run(obj,func,arg1,arg2,arg3,arg4));
        return ThreadRun::OK;
    }
    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3>
    ThreadRun::RC run(Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        new AlpmFuture<T>(this,QtConcurrent::run(obj,func,arg1,arg2,arg3));
        return ThreadRun::OK;
    }
    template<typename T, typename Class, typename Function, typename Arg1, typename Arg2>
    ThreadRun::RC run(Class obj,Function func,Arg1 arg1,Arg2 arg2) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        new AlpmFuture<T>(this,QtConcurrent::run(obj,func,arg1,arg2));
        return ThreadRun::OK;
    }
    template<typename T, typename Class, typename Function, typename Arg1>
    ThreadRun::RC run(Class obj,Function func,Arg1 arg1) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        new AlpmFuture<T>(this,QtConcurrent::run(obj,func,arg1));
        return ThreadRun::OK;
    }
    template<typename T, typename Class, typename Function>
    ThreadRun::RC run(Class obj,Function func) {
        if (isMethodExecuting()) return forbidden(true,func);
        m_method_name = methodName(func);
        new AlpmFuture<T>(this,QtConcurrent::run(obj,func));
        return ThreadRun::OK;
    }

    template<typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
    ThreadRun::RC run_void(Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4,Arg5 arg5) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        new AlpmFutureVoid(this,QtConcurrent::run(obj,func,arg1,arg2,arg3,arg4,arg5));
        return ThreadRun::OK;
    }
    template<typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    ThreadRun::RC run_void(Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3,Arg4 arg4) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        new AlpmFutureVoid(this,QtConcurrent::run(obj,func,arg1,arg2,arg3,arg4));
        return ThreadRun::OK;
    }
    template<typename Class, typename Function, typename Arg1, typename Arg2, typename Arg3>
    ThreadRun::RC run_void(Class obj,Function func,Arg1 arg1,Arg2 arg2,Arg3 arg3) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        new AlpmFutureVoid(this,QtConcurrent::run(obj,func,arg1,arg2,arg3));
        return ThreadRun::OK;
    }
    template<typename Class, typename Function, typename Arg1, typename Arg2>
    ThreadRun::RC run_void(Class obj,Function func,Arg1 arg1,Arg2 arg2) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        new AlpmFutureVoid(this,QtConcurrent::run(obj,func,arg1,arg2));
        return ThreadRun::OK;
    }
    template<typename Class, typename Function, typename Arg1>
    ThreadRun::RC run_void(Class obj,Function func,Arg1 arg1) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        new AlpmFutureVoid(this,QtConcurrent::run(obj,func,arg1));
        return ThreadRun::OK;
    }
    template<typename Class, typename Function>
    ThreadRun::RC run_void(Class obj,Function func) {
        if (isMethodExecuting()) return forbidden(false,func);
        m_method_name = methodName(func);
        new AlpmFutureVoid(this,QtConcurrent::run(obj,func));
        return ThreadRun::OK;
    }
protected:
    virtual ThreadRun::RC lastMethodRC() const = 0;

signals:
    void method_finished(const QString & name,const QVariant & result,ThreadRun::RC rc);
    void method_finished(const QString & name,ThreadRun::RC rc);

private:
    ThreadRun::RC forbidden(bool with_parms);

    template<typename Function>
    QString methodName(Function func) {
        QString ret;
        char ** strings = backtrace_symbols(reinterpret_cast<void **>(&func),1);
        if (strings == NULL) return ret;
        ret = QString::fromLocal8Bit((const char *)strings[0]);
        free(strings);
        int start = ret.indexOf('(');
        if (start < 0) return ret;
        int end = ret.indexOf('+',start);
        if (end < 0) return ret;
        char * dem_str = bfd_demangle(NULL,ret.mid(start+1,end - start - 1).toLocal8Bit().constData(),3);
        ret = QString::fromLocal8Bit(dem_str);
        free(dem_str);
        end = ret.indexOf('(',0);
        if (end < 0) return ret;
        return ret.mid(0,end);
    }
    template<typename T> void alpmFutureFinished(const T & result) {
       m_instance = NULL;
       emit method_finished(m_method_name,QVariant::fromValue<T>(result),isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC());
       m_method_name.clear();
    }
    void alpmFutureFinished() {
        m_instance = NULL;
        emit method_finished(m_method_name,isTerminateFlagSet()?ThreadRun::TERMINATED:lastMethodRC());
        m_method_name.clear();
    }
    template<typename Func> ThreadRun::RC forbidden(bool with_parms,Func func) {
        if (with_parms) emit method_finished(methodName(func),QVariant(),ThreadRun::FORBIDDEN);
        else emit method_finished(methodName(func),ThreadRun::FORBIDDEN);
        return ThreadRun::FORBIDDEN;
    }

    static bool m_terminate;
    static QObject * m_instance;
    static QString m_method_name;
};
Q_DECLARE_METATYPE(ThreadRun::RC)

#endif // ALPMFUTURE_H
