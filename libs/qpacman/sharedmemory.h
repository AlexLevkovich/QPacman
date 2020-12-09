/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <QString>
#include <QDateTime>
#include <QThread>
#include <QSystemSemaphore>
#include "inotifier.h"

class SharedMemory : public QObject {
    Q_OBJECT
public:
    SharedMemory(const QString & key);
    ~SharedMemory();
    // check size after open().
    //if it is 0 then it means the opened memory is new and you need using setSize() to set the size > 0 before attach().
    //the setting of username is possible if you are root
    bool open(size_t initial_size = 1,const QString & username = QString());
    bool close();
    QString errorString() const;
    //the using this function to set the data will result in absent changed() signal.
    //use setData() if you need that signal be received by other processes with the same semaphore open.
    const void * data();
    bool setData(const QByteArray & data,quint64 offset = 0);
    bool setData(const char * data,size_t size,quint64 offset = 0);
    QString key() const;
    size_t size() const;
    bool setSize(size_t val);
    bool isOpen() const;
    bool lock();
    bool unlock();
    QList<qint64> otherInstancesPids() const;

signals:
    void changed();

private slots:
    void slot_changed(const QString & path);
    void slot_path_attr_changed(const QString & path);
    void post_setting_size();

private:
    bool _attach();
    bool _detach();
    bool _setData(const char * data,size_t size,quint64 offset = 0);
    bool _unlink();
    bool _close(bool do_unlink = true);
    QList<qint64> _instancesPids(bool ignore_current = true) const;
    static bool someoneElseOpenedFile(const QString & _filename);

    QString m_key;
    int m_id;
    Inotifier shm_watcher;
    void * m_ptr;
    bool me_locked;
    QString m_key_filename;
    QString m_username;
    bool m_myevent;
    QSystemSemaphore m_lockSemaphore;
};

#endif // SHAREDMEMORY_H
