/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "sharedmemory.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>
#include <sys/types.h>
#include <pwd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SHM_DIR "/dev/shm"
#define CHANGE_TIMEOUT 1000

static int tokenStartIndex(const QString & str,const QChar & sep,int num) {
    if ((str.length() == 0) || (sep == '\0')) return -1;

    int idx = 0;
    for (int i=1;i<=num;i++) {
        idx = (i == 1)?0:str.indexOf(sep,idx);
        if (idx == -1) return -1;
        for (idx++;(idx < str.length()) && (str.at(idx) == sep);idx++) {}
        if (idx >= str.length()) return -1;
    }
    return idx;
}

SharedMemory::SharedMemory(const QString & key) : QObject(NULL) {
    m_key = key;
    m_ptr = NULL;
    m_id = -1;
    me_locked = false;
    m_myevent = false;

    errno = 0;
    m_key_filename = QString::fromLatin1(SHM_DIR) + QDir::separator() + m_key;

    if (!QDir(SHM_DIR).exists()) {
        fprintf(stderr,"Error: you need to enable the support for shared memory in kernel!\n");
        ::exit(66);
    }

    connect(&shm_watcher,SIGNAL(changed(const QString &)),SLOT(slot_changed(const QString &)));
    connect(&shm_watcher,SIGNAL(attributesChanged(const QString &)),SLOT(slot_path_attr_changed(const QString &)));
}

SharedMemory::~SharedMemory() {
    close();
}

bool SharedMemory::someoneElseOpenedFile(const QString & _filename)  {
    QString filename = QFileInfo(_filename).absoluteFilePath();

    QChar sep = QChar::fromLatin1(' ');
    QDirIterator pid_dirs("/proc",QDir::Dirs);
    bool ok;
    QString name;
    qint64 pid;
    QByteArray contents;
    while (pid_dirs.hasNext()) {
        pid_dirs.next();
        name = pid_dirs.fileName();
        pid = name.toLongLong(&ok);
        if (!ok) continue;
        QFile file(pid_dirs.filePath()+QDir::separator()+"maps");
        if (!file.open(QIODevice::ReadOnly)) continue;
        int idx;
        contents = file.readAll();
        file.close();
        QTextStream in(&contents);
        QString line;
        while (!in.atEnd()) {
            line = in.readLine();
            if (line.isEmpty()) continue;
            idx = tokenStartIndex(line,sep,6);
            if (idx == -1) continue;
            if ((filename == line.mid(idx)) && (QCoreApplication::applicationPid() != pid)) return true;
        }
    }

    return false;
}

QList<qint64> SharedMemory::otherInstancesPids() const {
    return _instancesPids();
}

QList<qint64> SharedMemory::_instancesPids(bool ignore_current) const {
    QList<qint64> pids;

    QChar sep = QChar::fromLatin1(' ');
    QDirIterator pid_dirs("/proc",QDir::Dirs);
    bool ok;
    QString name;
    qint64 pid;
    QByteArray contents;
    while (pid_dirs.hasNext()) {
        pid_dirs.next();
        name = pid_dirs.fileName();
        pid = name.toLongLong(&ok);
        if (!ok) continue;
        QFile file(pid_dirs.filePath()+QDir::separator()+"maps");
        if (!file.open(QIODevice::ReadOnly)) continue;
        int idx;
        contents = file.readAll();
        file.close();
        QTextStream in(&contents);
        QString line;
        while (!in.atEnd()) {
            line = in.readLine();
            if (line.isEmpty()) continue;
            idx = tokenStartIndex(line,sep,6);
            if (idx == -1) continue;
            if ((m_key_filename == line.mid(idx)) && (ignore_current?(QCoreApplication::applicationPid() != pid):true)) {
                pids.append(pid);
                break;
            }
        }
    }

    return pids;
}

QString SharedMemory::errorString() const {
    if (errno == 0) return QString();

    switch(errno) {
    case EACCES:
        return QObject::tr("Permission Exception.");
    case EEXIST:
        return QObject::tr("This shared memory object is already open.");
    case EADDRNOTAVAIL:
        return QObject::tr("Shared memory object specified by name is already open with the same PID.");
    case EINVAL:
        return QObject::tr("Invalid shared memory name passed or its size is zero.");
    case EMFILE:
        return QObject::tr("The process already has the maximum number of files open.");
    case ENAMETOOLONG:
        return QObject::tr("The length of name exceeds PATH_MAX.");
    case ENFILE:
        return QObject::tr("The limit on the total number of files open on the system has been reached.");
    case EBADF:
        return QObject::tr("Shared memory needs being created and/or attached.");
    case ENOBUFS:
        return QObject::tr("Shared memory is small for the buffer you are trying to set.");
    case ENOLCK:
        return QObject::tr("Other process locked the memory, so it needs to unlock it but not you!.");
    default:
        return QObject::tr("Invalid exception occurred in shared memory creation.");
    }
}

bool SharedMemory::open(size_t initial_size,const QString & username) {
    errno = 0;
    if (m_id != -1) {
        errno = EEXIST;
        return false;
    }

    if (_instancesPids(false).contains(QCoreApplication::applicationPid())) {
        errno = EADDRNOTAVAIL;
        return false;
    }

    m_username = username;
    if ((getuid() == 0) && !username.isEmpty()) chown((m_key_filename).toLocal8Bit().constData(),0,0);
    m_id = shm_open(m_key.toLocal8Bit().constData(), O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (m_id < 0) return false;

    if ((getuid() == 0) && !username.isEmpty()) {
        struct passwd * pw = getpwnam(username.toLocal8Bit().constData());
        if (pw != NULL) fchown(m_id,pw->pw_uid,pw->pw_gid);
    }

    if ((size() <= 0) && !setSize(initial_size)) {
        int save_errno = errno;
        close();
        errno = save_errno;
        return false;
    }

    if (!_attach()) {
        int save_errno = errno;
        close();
        errno = save_errno;
        return false;
    }

    shm_watcher.addPath(m_key_filename);

    return true;
}

bool SharedMemory::_attach() {
    if (m_id == -1) {
        errno = EBADF;
        return false;
    }

    size_t m_size = size();
    if (m_size == 0) {
        errno = EINVAL;
        return false;
    }

    errno = 0;
    return ((m_ptr = mmap(NULL,m_size,PROT_READ | PROT_WRITE,MAP_SHARED,m_id,0)) != MAP_FAILED);
}

bool SharedMemory::_detach() {
    if (m_ptr == NULL) {
        errno = EBADF;
        return false;
    }

    errno = 0;
    munmap(m_ptr, size());
    m_ptr = NULL;
    return true;
}

bool SharedMemory::_is_locked() {
    if (me_locked) return true;

    if ((::lockf(m_id,F_TEST,0) == -1) && (errno == EACCES || errno == EAGAIN)) {
        errno = 0;
        return true;
    }
    return false;
}

bool SharedMemory::lock() {
    if (m_id == -1) {
        errno = EINVAL;
        return false;
    }
    if (me_locked) return true;

    while (errno == EINTR) {
        if (::lockf(m_id,F_LOCK,0) == -1) {
            if (errno != EINTR) {
                if (errno == EDEADLK) {
                    fprintf(stderr,"Error: deadlock detected!\n");
                    ::usleep(100);
                    errno = EINTR;
                    continue;
                }
                return false;
            }
        }
        else break;
    }

    me_locked = true;

    return true;
}

bool SharedMemory::unlock() {
    if (m_id == -1) {
        errno = EINVAL;
        return false;
    }
    if (!me_locked) {
        if (_is_locked()) {
            errno = ENOLCK;
            return false;
        }
        return true;
    }

    if (::lockf(m_id,F_ULOCK,0) == -1) return false;

    me_locked = false;

    return true;
}

const void * SharedMemory::data() {
    return m_ptr;
}

bool SharedMemory::setData(const QByteArray & data,quint64 offset) {
    return _setData(data.constData(),data.length(),offset);
}

bool SharedMemory::setData(const char * data,size_t size,quint64 offset) {
    return _setData(data,size,offset);
}

bool SharedMemory::_setData(const char * data,size_t size,quint64 offset) {
    bool locked = me_locked;
    if (!lock()) return false;
    if (!setSize((size_t)(offset+size))) {
        if (!locked) unlock();
        return false;
    }
    memcpy(((char *)m_ptr)+offset,data,size);
    QMetaObject::invokeMethod(this,"post_setting_size",Qt::QueuedConnection);
    if (!locked) unlock();

    return true;
}

void SharedMemory::post_setting_size() {
    bool locked = me_locked;
    if (!lock()) return;
    setSize(size());
    if (!locked) unlock();
}

void SharedMemory::slot_changed(const QString &) {
    if (m_myevent) {
        m_myevent = false;
        return;
    }

    emit changed();
}

QString SharedMemory::key() const {
    return m_key;
}

size_t SharedMemory::size() const {
    if (m_id == -1) {
        errno = EBADF;
        return 0;
    }

    /*struct stat st;
    if (fstat(m_id,&st) == -1) return 0;

    return st.st_size;*/
    size_t pos = lseek(m_id,0,SEEK_CUR);
    size_t size = lseek(m_id,0,SEEK_END);
    lseek(m_id,pos,SEEK_SET);

    return size;
}

bool SharedMemory::setSize(size_t val) {
    if (m_id == -1) {
        errno = EBADF;
        return false;
    }

    bool was_attached = _detach();
    bool locked = me_locked;
    if (!lock()) return false;
    errno = 0;
    m_myevent = true;
    if (ftruncate(m_id, val) < 0) {
        int old_errno = errno;
        if (!locked) unlock();
        errno = old_errno;
        return false;
    }
    if (!locked) unlock();

    return was_attached?_attach():true;
}

bool SharedMemory::isOpen() const {
    return (m_ptr != NULL);
}

bool SharedMemory::_close(bool do_unlink) {
    if (m_id == -1) {
        errno = EBADF;
        return false;
    }

    if (me_locked) unlock();
    _detach();
    ::close(m_id);
    m_id = -1;
    errno = 0;
    shm_watcher.removeAllPaths();
    if (do_unlink) _unlink();

    return true;
}

bool SharedMemory::close() {
    return _close();
}

bool SharedMemory::_unlink() {
    if (someoneElseOpenedFile(m_key_filename)) return true;
    return (shm_unlink(m_key.toLocal8Bit().constData()) == 0);
}

void SharedMemory::slot_path_attr_changed(const QString & path) {
    while (true) {
        struct stat st;
        struct stat ist;
        int ret;
        ret = stat(path.toLocal8Bit().constData(),&st);
        if ((ret == -1) && errno == ENOENT) break;
        if (ret == -1) return;
        if (fstat(m_id,&ist) == -1) return;
        if (st.st_ino != ist.st_ino) break;
        return;
    }

    bool locked = me_locked;
    if (!lock()) return;
    QFile file(path);
    if (!file.exists()) {
        qInfo() << "recreating" << path << "from the buffer!";
        size_t len = size();
        if (file.open(QIODevice::WriteOnly)) {
            if (len > 0) {
                size_t written = 0;
                qint64 part_written;
                while (written < len) {
                    part_written = file.write((const char *)data()+written,len-written);
                    if (part_written < 0) {
                        file.close();
                        file.remove();
                        break;
                    }
                    written += part_written;
                }
                if (file.isOpen()) qInfo() << "recreated!";
            }
            file.close();
        }
    }
    if (file.exists() && file.isWritable()) {
        qInfo() << "closing and reopening the file...";
        _close(false);
        open(1,m_username);
    }
    if (!locked) unlock();
}
