/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef INOTIFIER_H
#define INOTIFIER_H

#include <QSocketNotifier>
#include <QDir>
#include <QTime>
#include <QSet>
#include <QMap>
#include <QHash>
#include <sys/inotify.h>
#include <unistd.h>


class Inotifier : public QObject {
    Q_OBJECT
public:
    enum Error {
        NoReadAccess = EACCES,
        NotAccessable = EFAULT,
        LongPathName = ENAMETOOLONG,
        Invalid = EINVAL,
        DoesNotExists = ENOENT,
        InsufficientMemory = ENOMEM,
        UserLimitReached = ENOSPC,
        MustBeDir = ENOTDIR,
        Duplicate = EEXIST
    };

    Inotifier();
    ~Inotifier();

    void addPath(const QString &path);
    void addFolder(const QString &path);
    void removePathStartingWith(const QString &path);
    void removeAllPaths();
    int count() const;
    QStringList paths() const;

private:
    bool findSubFolders(const QDir &dir,QStringList &fullList);
    void inotifyRegisterPath(const QString &path);
    void changeDetected(const QString &path);

private slots:
    void receivedNotification(int fd);

signals:
    void exhausted();
    void attributesChanged(const QString & path);
    void changed(const QString & path);
    void rejected(const QString & path,Inotifier::Error error);

private:
    QSocketNotifier * m_notifier;
    int m_fd;
    bool m_isExhausted;
    QHash<int, QString> m_watchToPath;
    QMap<QString, int> m_pathToWatch;
};

#endif // INOTIFIER_H
