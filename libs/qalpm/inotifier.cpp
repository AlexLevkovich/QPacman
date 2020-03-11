/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "inotifier.h"
#include <QDebug>

Inotifier::Inotifier() {
    m_isExhausted = true;
    m_fd = inotify_init1(IN_NONBLOCK);
    if (m_fd != -1) {
        m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read);
        connect(m_notifier,SIGNAL(activated(int)),this,SLOT(receivedNotification(int)));
    } else {
        qCritical() << "Inotifier" << "notify_init1() failed: " << strerror(errno);
    }
}

Inotifier::~Inotifier() {
    if (m_fd != -1) {
        delete m_notifier;
        ::close(m_fd);
    }
}

bool Inotifier::findSubFolders(const QDir &dir, QStringList &fullList) {
    if (!(dir.exists() && dir.isReadable())) {
        qWarning() << "Inotifier" << "Non existing path coming in: " << dir.absolutePath();
        return false;
    } else {
        QStringList nameFilter;
        nameFilter << QLatin1String("*");
        QDir::Filters filter = QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden;
        const QStringList pathes = dir.entryList(nameFilter, filter);

        QStringList::const_iterator constIterator;
        for (constIterator = pathes.constBegin(); constIterator != pathes.constEnd();++constIterator) {
            const QString fullPath(dir.path() + QDir::separator() + (*constIterator));
            fullList.append(fullPath);
            if(!findSubFolders(QDir(fullPath),fullList)) return false;
        }
    }

    return true;
}

void Inotifier::inotifyRegisterPath(const QString &path) {
    if (path.isEmpty()) return;

    if (m_fd == -1) {
        qInfo() << "Inotifier" << "path is rejected:" << path;
        emit rejected(path,Inotifier::Invalid);
        return;
    }

    int wd = inotify_add_watch(m_fd,path.toLocal8Bit().constData(),QFileInfo(path).isDir()?(IN_CREATE | IN_DELETE | IN_CLOSE | IN_ATTRIB | IN_MOVE |
                                                                                            IN_DELETE_SELF | IN_MOVE_SELF | IN_UNMOUNT | IN_ONLYDIR):
                                                                                            (IN_MODIFY | IN_UNMOUNT | IN_ATTRIB));
    if (wd > -1) {
        m_watchToPath.insert(wd, path);
        m_pathToWatch.insert(path, wd);
    } else {
        // If we're running out of memory or inotify watches, become
        // unreliable.
        if (m_isExhausted && (errno == ENOMEM || errno == ENOSPC)) {
            m_isExhausted = false;
            emit exhausted();
        }
        else {
            qInfo() << "Inotifier" << "path is rejected:" << path;
            emit rejected(path,(Inotifier::Error)errno);
        }
    }
}

void Inotifier::addPath(const QString &path) {
    QString inPath = QDir(path).absolutePath();
    if (m_pathToWatch.contains(inPath)) return;

    qInfo() << "Inotifier" << "(+):" << inPath;
    inotifyRegisterPath(inPath);
}

void Inotifier::addFolder(const QString &path) {
    QString inPath = QDir(path).absolutePath();
    if (m_pathToWatch.contains(inPath)) return;

    qInfo() << "Inotifier" << "(+):" << inPath;

    inotifyRegisterPath(inPath);

    QStringList allSubfolders;
    findSubFolders(QDir(inPath), allSubfolders);
    QStringListIterator subfoldersIt(allSubfolders);
    while (subfoldersIt.hasNext()) {
        QString subfolder = subfoldersIt.next();
        QDir folder(subfolder);
        if (folder.exists() && !m_pathToWatch.contains(folder.absolutePath())) {
            if (subfolder.isEmpty()) {
                qDebug() << "Inotifier" << "Ignoring empty folder:" << folder.path();
                continue;
            }
            inotifyRegisterPath(folder.absolutePath());
        }
        else qWarning() << "Inotifier" << "Folder does not exist or is duplicate:" << folder.path();
    }
}

int Inotifier::count() const {
    return m_pathToWatch.count();
}

QStringList Inotifier::paths() const {
    return m_pathToWatch.keys();
}

void Inotifier::receivedNotification(int fd) {
    int len;
    const struct inotify_event *event = NULL;
    int buf_len = 4096;
    char * buffer = (char * )malloc(buf_len);
    char *ptr;
    int ret;

    errno = 0;
    len = 0;
    for (;errno != EAGAIN;) {
        do {
            ret = ::read(fd, buffer+len, buf_len-len);
            if ((ret == -1) && (errno != EAGAIN)) {
                if (errno == EINVAL) {
                    buf_len *= 2;
                    buffer = (char * )realloc(buffer,buf_len);
                    continue;
                }
                else {
                    qCritical() << "Inotifier" << "read() failed: " << strerror(errno);
                    free(buffer);
                    return;
                }
            }
        } while (false);
        if (ret != -1) len += ret;
    }

    for (ptr = buffer; ptr < buffer + len;ptr += sizeof(struct inotify_event) + event->len) {
        event = (const struct inotify_event *)ptr;
        if (event == NULL) continue;

        QString p = m_watchToPath[event->wd] + ((event->len > 0)?('/' + QString::fromLocal8Bit(event->name)):QString());
        if (event->mask & IN_IGNORED) {
            if (p == m_watchToPath[event->wd]) {
                m_pathToWatch.remove(m_watchToPath[event->wd]);
                m_watchToPath.remove(event->wd);
            }
            if (!p.isEmpty()) {
                qInfo() << "Inotifier" << "path is rejected:" << p;
                emit rejected(p,Inotifier::DoesNotExists);
            }
        }
        else if (event->mask & IN_ATTRIB) {
            qInfo() << "Inotifier" << "path attributes are changed:" << p;
            emit attributesChanged(p);
        }
        else {
            changeDetected(p);
            if ((event->mask & (IN_MOVED_TO | IN_CREATE))
                && QFileInfo(p).isDir()
                && !p.isEmpty()) addFolder(p);
            if ((event->mask & (IN_MOVED_FROM | IN_DELETE))
                && QFileInfo(p).isDir()) removePathStartingWith(p);
        }
    }

    free(buffer);
}

void Inotifier::changeDetected(const QString &path) {
    if (path.isEmpty()) return;

    qInfo() << "Inotifier" << "Detected changes in path:" << path;
    emit changed(path);
}

void Inotifier::removePathStartingWith(const QString &path) {
    QMap<QString,int>::Iterator it = m_pathToWatch.find(path);
    if (it == m_pathToWatch.end()) return;

    QString pathSlash = path + '/';

    // Remove the entry and all subentries
    while (it != m_pathToWatch.end()) {
        QString itPath = it.key();
        if (!itPath.startsWith(path)) break;
        if (itPath != path && !itPath.startsWith(pathSlash)) {
            ++it;
            continue;
        }

        int wid = it.value();
        inotify_rm_watch(m_fd, wid);
        m_watchToPath.remove(wid);
        it = m_pathToWatch.erase(it);
        qInfo() << "Inotifier" << "(-):" << itPath;
    }
}

void Inotifier::removeAllPaths() {
    if (m_pathToWatch.count() <= 0) return;
    QString key = m_pathToWatch.begin().key();
    removePathStartingWith(key);
}
