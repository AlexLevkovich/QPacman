/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "singleapplicationevents.h"
#include <QCoreApplication>
#include <QDataStream>
#include <QByteArray>
#include <QDateTime>
#include <QIODevice>
#include <QDebug>
#include <QDir>
#include <QProcessEnvironment>

const QByteArray SingleApplicationEvents::shared_id = QByteArray("QPSHM");
const QString SingleApplicationEvents::shared_key = QString("qpacman_singleapplication");

#define ENTRY_TIMEOUT 10000

class BaseEntry {
public:
    enum EntryType {
        START = 0,
        END = 1
    };

    virtual EntryType type() = 0;
    virtual QString pgmName() const = 0;
    virtual qint64 PID() const = 0;

    virtual bool add(QDataStream & stream) = 0;
    virtual bool add(SharedMemory * p_shm) = 0;
    virtual bool get(QDataStream & stream,bool & wrong_type) = 0;
    virtual bool isValid() const = 0;
    virtual void makeLessValid(qint64 pid) = 0;
    virtual bool isOld() const = 0;
    bool isZero() const {
        return (pgmName().isEmpty() && (PID() == 0));
    }

    QByteArray shmData(SharedMemory * p_shm,bool & ok);

    static qint64 readCounter(QDataStream & stream) {
        qint64 counter;
        QByteArray arr = stream.device()->read(sizeof(qint64));
        if (arr.length() != sizeof(qint64)) return 0;
        memcpy((char *)&counter,arr.constData(),sizeof(qint64));
        return counter;
    }
};

class StartEntry : public BaseEntry {
public:
    StartEntry(const QStringList & args,qint64 pid,const QList<qint64> & pids) {
        m_args = args;
        this->pid = pid;
        this->pids = pids;
        this->timestamp = QDateTime::currentDateTime();
    }

    StartEntry() {
        pid = -1;
    }

    virtual ~StartEntry() {}

    EntryType type() {
        return START;
    }

    bool add(SharedMemory * p_shm) {
        bool ok;
        QByteArray data = shmData(p_shm,ok);
        if (!ok) return false;

        QDataStream stream(&data,QIODevice::WriteOnly|QIODevice::Append);
        if (!add(stream)) return false;

        return p_shm->setData(data);
    }

    bool add(QDataStream & stream) {
        stream << (short)type();
        if (stream.status() == QDataStream::WriteFailed) return false;
        stream << m_args;
        if (stream.status() == QDataStream::WriteFailed) return false;
        stream << pid;
        if (stream.status() == QDataStream::WriteFailed) return false;
        stream << pids;
        if (stream.status() == QDataStream::WriteFailed) return false;
        stream << timestamp;
        if (stream.status() == QDataStream::WriteFailed) return false;
        return true;
    }

    bool get(QDataStream & stream,bool & wrong_type) {
        qint64 old_pos = stream.device()->pos();
        wrong_type = false;
        short m_type;
        stream >> m_type;
        if (stream.status() != QDataStream::Ok) return false;
        if (type() != (EntryType)m_type) {
            wrong_type = true;
            stream.device()->seek(old_pos);
            return false;
        }
        stream >> m_args;
        if (stream.status() != QDataStream::Ok) return false;
        stream >> pid;
        if (stream.status() != QDataStream::Ok) return false;
        stream >> pids;
        if (stream.status() != QDataStream::Ok) return false;
        stream >> timestamp;
        return (stream.status() == QDataStream::Ok);
    }

    QString pgmName() const {
        return m_args[0];
    }

    QStringList arguments() const {
        if (m_args.count() <= 1) return QStringList();
        return m_args.mid(1);
    }

    qint64 PID() const {
        return pid;
    }

    bool isValid() const {
        return ((pid > 0) && !pids.isEmpty());
    }

    bool isRunning() const {
        return ((pid > 0) && QDir(QString::fromLatin1("/proc/%1").arg(pid)).exists());
    }

    bool isOld() const {
        return ((pid <= 0) || (timestamp.msecsTo(QDateTime::currentDateTime()) >= ENTRY_TIMEOUT));
    }

    void makeLessValid(qint64 pid) {
        pids.removeAll(pid);
    }

private:
    QStringList m_args;
    qint64 pid;
    QList<qint64> pids;
    QDateTime timestamp;
};

class EndEntry : public BaseEntry {
public:
    EndEntry(const QStringList & args,qint64 pid,int rc,const QList<qint64> & pids) {
        this->m_args = args;
        this->pid = pid;
        this->rc = rc;
        this->pids = pids;
        this->timestamp = QDateTime::currentDateTime();
    }

    EndEntry() {
        pid = -1;
        rc = -1;
    }

    EntryType type() {
        return END;
    }

    virtual ~EndEntry() {}

    bool add(SharedMemory * p_shm) {
        bool ok;
        QByteArray data = shmData(p_shm,ok);
        if (!ok) return false;

        QDataStream stream(&data,QIODevice::WriteOnly|QIODevice::Append);
        if (!add(stream)) return false;

        return p_shm->setData(data);
    }

    bool add(QDataStream & stream) {
        stream << (short)type();
        if (stream.status() == QDataStream::WriteFailed) return false;
        stream << m_args;
        if (stream.status() == QDataStream::WriteFailed) return false;
        stream << pid;
        if (stream.status() == QDataStream::WriteFailed) return false;
        stream << rc;
        if (stream.status() == QDataStream::WriteFailed) return false;
        stream << pids;
        if (stream.status() == QDataStream::WriteFailed) return false;
        stream << timestamp;
        if (stream.status() == QDataStream::WriteFailed) return false;
        return true;
    }

    bool get(QDataStream & stream,bool & wrong_type) {
        qint64 old_pos = stream.device()->pos();
        wrong_type = false;
        short m_type;
        stream >> m_type;
        if (stream.status() != QDataStream::Ok) return false;
        if (type() != (EntryType)m_type) {
            wrong_type = true;
            stream.device()->seek(old_pos);
            return false;
        }
        stream >> m_args;
        if (stream.status() != QDataStream::Ok) return false;
        stream >> pid;
        if (stream.status() != QDataStream::Ok) return false;
        stream >> rc;
        if (stream.status() != QDataStream::Ok) return false;
        stream >> pids;
        if (stream.status() != QDataStream::Ok) return false;
        stream >> timestamp;
        return (stream.status() == QDataStream::Ok);
    }

    QString pgmName() const {
        return m_args[0];
    }

    QStringList arguments() const {
        if (m_args.count() <= 1) return QStringList();
        return m_args.mid(1);
    }

    qint64 PID() const {
        return pid;
    }

    int RC() const {
        return rc;
    }

    bool isValid() const {
        return ((pid > 0) && !pids.isEmpty());
    }

    bool isOld() const {
        return ((pid <= 0) || (timestamp.msecsTo(QDateTime::currentDateTime()) >= ENTRY_TIMEOUT));
    }

    void makeLessValid(qint64 pid) {
        pids.removeAll(pid);
    }

private:
    QStringList m_args;
    qint64 pid;
    int rc;
    QList<qint64> pids;
    QDateTime timestamp;
};

static bool shmData(SharedMemory * p_shm,QList<StartEntry> & start_entries,QList<EndEntry> & end_entries) {
    StartEntry start_entry;
    EndEntry end_entry;
    bool wrong_type;
    QByteArray data((const char *)p_shm->data(),p_shm->size());
    QDataStream stream(&data,QIODevice::ReadOnly);
    if (stream.device()->read(SingleApplicationEvents::shared_id.length()) != SingleApplicationEvents::shared_id) return false;
    for (;!stream.atEnd();) {
        if (!start_entry.get(stream,wrong_type)) {
            if (wrong_type) {
                if (!end_entry.get(stream,wrong_type)) return false;
                if (end_entry.isZero()) break;
                if (!end_entry.isValid()) continue;
                if (end_entry.isOld()) continue;
                end_entries.append(end_entry);
            }
            else return false;
        }
        else {
            if (start_entry.isZero()) break;
            start_entries.append(start_entry);
        }
    }
    return true;
}

QByteArray BaseEntry::shmData(SharedMemory * p_shm,bool & ok) {
    QList<StartEntry> start_entries;
    QList<EndEntry> end_entries;
    ok = ::shmData(p_shm,start_entries,end_entries);
    if (!ok) return SingleApplicationEvents::shared_id;

    QByteArray arr = SingleApplicationEvents::shared_id;
    QDataStream stream(&arr,QIODevice::WriteOnly|QIODevice::Append);
    if (start_entries.count() > 0) {
        for (int i=0;i<start_entries.count();i++) {
            if (!start_entries[i].add(stream)) {
                ok = false;
                return SingleApplicationEvents::shared_id;
            }
        }
    }
    if (end_entries.count() > 0) {
        for (int i=0;i<end_entries.count();i++) {
            if (!end_entries[i].add(stream)) {
                ok = false;
                return SingleApplicationEvents::shared_id;
            }
        }
    }
    return arr;
}

SingleApplicationEvents::SingleApplicationEvents(int &argc,char **argv,QObject *parent) : QObject(parent),
                                                                                          shm_obj(shared_key) {
    QString orig_user = QProcessEnvironment::systemEnvironment().value("ORIGINAL_USER","");
    if (!shm_obj.open(1,((getuid() == 0) && !orig_user.isEmpty())?orig_user:QString())) {
        fprintf(stderr,"%s\n",shm_obj.errorString().toLocal8Bit().constData());
        shm_obj.close();
        ::exit(67);
        return;
    }
    if ((shm_obj.size() == 1) && !shm_obj.setData(SingleApplicationEvents::shared_id)) {
        fprintf(stderr,"%s\n",shm_obj.errorString().toLocal8Bit().constData());
        shm_obj.close();
        ::exit(68);
        return;
    }
    connect(&shm_obj,SIGNAL(changed()),SLOT(shm_changed()));

    addStartEntry(argc,argv);
}

void SingleApplicationEvents::addExitEntry(int rc) {
    if (m_args.count() <= 0) {
        fprintf(stderr,"Error: pgm_path wasn't set!!!\n");
        QCoreApplication::exit(70);
    }

    if (!shm_obj.lock()) return;
    EndEntry(m_args,pid,rc,shm_obj.otherInstancesPids()).add(&shm_obj);
    shm_obj.unlock();
}

void SingleApplicationEvents::addStartEntry(int &argc,char **argv) {
    if (argc < 1 || argv == NULL) {
        fprintf(stderr,"Error: incorrect count of input parameters!!!\n");
        shm_obj.close();
        ::exit(70);
    }

    m_args.clear();
    for (int i=0;i<argc;i++) {
        m_args.append(QString::fromLocal8Bit(argv[i]));
    }
    pid = QCoreApplication::applicationPid();

    if (isOtherInstanceAlreadyStarted()) QMetaObject::invokeMethod(this,"secondInstanceIAm",Qt::QueuedConnection);
    if (!shm_obj.lock()) return;
    StartEntry(m_args,pid,shm_obj.otherInstancesPids()).add(&shm_obj);
    shm_obj.unlock();
}

void SingleApplicationEvents::shm_changed() {
    if (!shm_obj.lock()) return;

    QList<StartEntry> start_entries;
    QList<EndEntry> end_entries;
    if (!::shmData(&shm_obj,start_entries,end_entries)) {
        shm_obj.unlock();
        return;
    }

    int i;
    for (i=0;i<start_entries.count();i++) {
        if (!start_entries[i].isValid()) continue;
        start_entries[i].makeLessValid(QCoreApplication::applicationPid());
        if ((m_args[0] == start_entries[i].pgmName()) && (pid != start_entries[i].PID())) QMetaObject::invokeMethod(this,"secondInstanceStarted",Qt::QueuedConnection,Q_ARG(QStringList,start_entries[i].arguments()),Q_ARG(qint64,start_entries[i].PID()));
        else QMetaObject::invokeMethod(this,"applicationStarted",Qt::QueuedConnection,Q_ARG(QString,start_entries[i].pgmName()),Q_ARG(QStringList,start_entries[i].arguments()),Q_ARG(qint64,start_entries[i].PID()));
    }
    for (i=0;i<end_entries.count();i++) {
        end_entries[i].makeLessValid(QCoreApplication::applicationPid());
        QMetaObject::invokeMethod(this,"applicationExited",Qt::QueuedConnection,Q_ARG(QString,end_entries[i].pgmName()),Q_ARG(QStringList,end_entries[i].arguments()),Q_ARG(qint64,end_entries[i].PID()),Q_ARG(int,end_entries[i].RC()));
    }

    QByteArray data = SingleApplicationEvents::shared_id;
    QDataStream stream(&data,QIODevice::WriteOnly|QIODevice::Append);
    for (i=0;i<start_entries.count();i++) {
        if ((start_entries[i].isValid() || start_entries[i].isRunning() || !start_entries[i].isOld()) && !start_entries[i].add(stream)) {
            data = SingleApplicationEvents::shared_id;
            break;
        }
    }
    for (i=0;i<end_entries.count();i++) {
        if (!end_entries[i].add(stream)) {
            data = SingleApplicationEvents::shared_id;
            break;
        }
    }
    shm_obj.setData(data);

    shm_obj.unlock();
}

bool SingleApplicationEvents::isApplicationStarted(const QString & appname) {
    if (!shm_obj.lock()) return false;

    QList<StartEntry> start_entries;
    QList<EndEntry> end_entries;
    if (!::shmData(&shm_obj,start_entries,end_entries)) {
        shm_obj.unlock();
        return false;
    }

    for (int i=0;i<start_entries.count();i++) {
        if (start_entries[i].isRunning() && (QFileInfo(start_entries[i].pgmName()).fileName() == appname)) {
            shm_obj.unlock();
            return true;
        }
    }

    shm_obj.unlock();
    return false;
}

bool SingleApplicationEvents::isOtherInstanceAlreadyStarted() {
    if (!shm_obj.lock()) return false;

    QList<StartEntry> start_entries;
    QList<EndEntry> end_entries;
    if (!::shmData(&shm_obj,start_entries,end_entries)) {
        shm_obj.unlock();
        return false;
    }

    for (int i=0;i<start_entries.count();i++) {
        if (start_entries[i].isRunning() && (start_entries[i].pgmName() == m_args[0]) && (start_entries[i].PID() != pid)) {
            shm_obj.unlock();
            return true;
        }
    }

    shm_obj.unlock();
    return false;
}
