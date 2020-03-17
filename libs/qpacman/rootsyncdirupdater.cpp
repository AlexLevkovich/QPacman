#include "rootsyncdirupdater.h"
#include "alpmlockingnotifier.h"
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QDateTime>
#include <QCryptographicHash>
#include <QDirIterator>
#include <static.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "alpmdb.h"
#include "libalpm.h"
#include "downloaderinterface.h"

RootSyncDirUpdater::RootSyncDirUpdater(const QString & sysSyncDir,const QString & userSyncDir,AlpmLockingNotifier * locking_notifier,QObject *parent) : QEventLoop(parent) {
    m_locking_notifier = locking_notifier;
    m_sysSyncDir = sysSyncDir;
    m_userSyncDir = userSyncDir;
    connect(this,&RootSyncDirUpdater::error,[&](const QString & error) {
        qDebug() << "copying userSyncDir to sysSyncDir: " + error;
        m_error = error;
        quit();
    });

    QMetaObject::invokeMethod(this,"process",Qt::QueuedConnection);
}

void RootSyncDirUpdater::process() {
    QDir userSyncDir(m_userSyncDir);
    QDir sysSyncDir(m_sysSyncDir);
    if (m_userSyncDir.isEmpty() || !userSyncDir.exists()) {
        emit error(QString::fromLatin1("RootSyncDirUpdater: ") + tr("userSyncDir variable is empty or the dir it represents does not exist!!!"));
        return;
    }
    if (m_sysSyncDir.isEmpty() || !sysSyncDir.exists()) {
        emit error(QString::fromLatin1("RootSyncDirUpdater: ") + tr("sysSyncDir variable is empty or the dir it represents does not exist!!!"));
        return;
    }

    if (isLeftDirNewer(userSyncDir,sysSyncDir)) {
        qDebug() << "copying userSyncDir to sysSyncDir...";
        updateSystemSyncDir(sysSyncDir,userSyncDir);
    }
    emit ok();
    quit();
}

class LockFileArranger {
private:
    LockFileArranger(const QString & name) {
        m_name = name;
        if (!QFile(name).open(QIODevice::WriteOnly)) m_name.clear();
    }
    ~LockFileArranger() {
        if (isCreated()) QFile(m_name).remove();
    }
    bool isCreated() const {
        return !m_name.isEmpty();
    }

    QString m_name;
    friend class RootSyncDirUpdater;
};

class SignalBlocker {
private:
    SignalBlocker(QObject * obj = NULL) {
        was_blocked = false;
        setObject(obj);
    }
    ~SignalBlocker() {
        if (m_obj != NULL && !was_blocked) m_obj->blockSignals(false);
    }
    void setObject(QObject * obj) {
        m_obj = obj;
        if (m_obj != NULL) {
            was_blocked = m_obj->signalsBlocked();
            if (!was_blocked) m_obj->blockSignals(true);
        }
    }

    QObject * m_obj;
    bool was_blocked;
    friend class RootSyncDirUpdater;
};

void RootSyncDirUpdater::updateSystemSyncDir(const QDir & sys_sync_path,const QDir & user_sync_path) {
    QStringList lock_files_to_wait;
    QString sys_lock_file = sys_sync_path.path()+QDir::separator()+".." + QDir::separator() + "db.lck";
    if (QFile(sys_lock_file).exists()) lock_files_to_wait.append(sys_lock_file);
    QString user_lock_file = user_sync_path.path()+QDir::separator()+".." + QDir::separator() + "db.lck";
    if (QFile(user_lock_file).exists()) lock_files_to_wait.append(user_lock_file);
    SignalBlocker blocker;
    if (m_locking_notifier != NULL) blocker.setObject(m_locking_notifier);
    if (lock_files_to_wait.count() > 0) emit locked_files(lock_files_to_wait);
    LockFileArranger sys_lock(sys_lock_file);
    if (!sys_lock.isCreated()) {
        emit error(tr("Something wrong: cannot create lock file: ") + sys_lock_file);
        return;
    }
    LockFileArranger user_lock(user_lock_file);
    if (user_lock.isCreated()) {
        QString orig_user = QProcessEnvironment::systemEnvironment().value("ORIGINAL_USER","");
        if (orig_user.isEmpty()) {
            emit error(QString::fromLatin1("updateSystemSyncDir: ") + tr("Something wrong with application setup, ask programmer what to do"));
            QCoreApplication::exit(103);
            return;
        }
        struct passwd * pw = getpwnam(orig_user.toLocal8Bit().constData());
        if (pw == NULL) {
            emit error(QString::fromLatin1("updateSystemSyncDir: ") + tr("something wrong with application setup, ask programmer what to do"));
            return;
        }
        chown(user_lock_file.toLocal8Bit().constData(),pw->pw_uid,pw->pw_gid);
    }
    else {
        emit error(tr("Something wrong: cannot create user lock file: ") + user_lock_file);
        return;
    }
    copyDirectoryFiles(user_sync_path.path(),sys_sync_path.path(),"db");
}

bool RootSyncDirUpdater::copyDirectoryFiles(const QString & fromDir,const QString & toDir,const QString & suffix) {
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    if(!targetDir.exists() && !targetDir.mkdir(targetDir.absolutePath())) {
        emit error(QString::fromLatin1("copyDirectoryFiles: ") + tr("cannot create dir: ") + targetDir.path());
        return false;
    }
    if(!sourceDir.exists()) {
        emit error(QString::fromLatin1("copyDirectoryFiles: ") + tr("source dir does not exist: ") + fromDir);
        return false;
    }

    QString targetPath;
    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach (QFileInfo fileInfo, fileInfoList) {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..") continue;

        if(fileInfo.isDir()) {
            if(!copyDirectoryFiles(fileInfo.filePath(),targetDir.filePath(fileInfo.fileName()))) return false;
        }
        else {
            if (!suffix.isEmpty() && fileInfo.suffix() != suffix) continue;
            if(targetDir.exists(fileInfo.fileName())) targetDir.remove(fileInfo.fileName());
            targetPath = targetDir.filePath(fileInfo.fileName());
            if(!QFile::copy(fileInfo.filePath(),targetPath)) {
                emit error(QString::fromLatin1("copyDirectoryFiles: ") + tr("cannot copy ") + fileInfo.filePath() + tr(" to ") + targetPath);
                return false;
            }
            DownloaderInterface::setFileDate(targetPath,fileInfo.lastModified());
        }
    }

    qDebug() << "ok copying userSyncDir to sysSyncDir...";
    emit need_alpm_reload();

    return true;
}

int RootSyncDirUpdater::have_file_with_same_name(const QStringList & arr,const QString & name) {
    if (arr.count() <= 0) return -1;

    for (int i=0;i<arr.count();i++) {
        if (QFileInfo(arr.at(i)).fileName() == name) return i;
    }

    return -1;
}

qint64 RootSyncDirUpdater::compare_file_dates(const QString & left,const QString & right) {
    return QFileInfo(left).fileTime(QFileDevice::FileModificationTime).toMSecsSinceEpoch() - QFileInfo(right).fileTime(QFileDevice::FileModificationTime).toMSecsSinceEpoch();
}

int RootSyncDirUpdater::compare_file_sizes(const QString & left,const QString & right) {
    return md5_summ(left).compare(md5_summ(right));
}

const QStringList RootSyncDirUpdater::dirContents(const QDir & dir,const QString & nameFilter) {
    QStringList ret;
    QDirIterator it(dir.path(),QStringList() << nameFilter);
    while (it.hasNext()) {
        ret << it.next();
    }
    return ret;
}

bool RootSyncDirUpdater::isLeftDirNewer(const QDir & left,const QDir & right) {
    QString ext = AlpmDB::extension();
    QStringList left_files = leave_only_real_db_files(dirContents(left,"*"+ext));
    QStringList right_files = leave_only_real_db_files(dirContents(right,"*"+ext));
    if (Alpm::instance() == NULL) return false;

    QList<AlpmDB> dbs = Alpm::instance()->allSyncDBs();
    QStringList expected_names;
    int i;
    for (i=0;i<dbs.count();i++) {
        expected_names.append(dbs[i].name()+ext);
    }

    QList<int> left_files_idxs;
    QList<int> right_files_idxs;
    int idx;
    for (i=0;i<expected_names.count();i++) {
        if ((idx = have_file_with_same_name(left_files,expected_names.at(i))) < 0) return false;
        left_files_idxs.append(idx);
        if ((idx = have_file_with_same_name(right_files,expected_names.at(i))) < 0) return true;
        right_files_idxs.append(idx);
    }

    bool same = true;
    for (i=(left_files_idxs.count()-1);i>=0;i--) {
        if (compare_file_sizes(left_files.at(left_files_idxs.at(i)),right_files.at(right_files_idxs.at(i))) != 0) {
            same = false;
        }
        else {
            left_files_idxs.removeAt(i);
            right_files_idxs.removeAt(i);
        }
    }
    if (same) return false;

    for (i=0;i<left_files_idxs.count();i++) {
        if (compare_file_dates(left_files.at(left_files_idxs.at(i)),right_files.at(right_files_idxs.at(i))) > 0) {
            return true;
        }
    }

    return false;
}

const QStringList RootSyncDirUpdater::leave_only_real_db_files(const QStringList & list) {
    if (!Alpm::isOpen()) return QStringList();

    QStringList ret_list = list;
    QStringList db_names;
    QList<AlpmDB> list_dbs = Alpm::instance()->allSyncDBs();
    int i;
    QString ext = AlpmDB::extension();
    for (i=0;i<list_dbs.count();i++) {
        db_names << list_dbs[i].name()+ext;
    }
    for (i=(list.count()-1);i>=0;i--) {
        if (!db_names.contains(QFileInfo(list.at(i)).fileName())) ret_list.removeAt(i);
    }
    return ret_list;
}

const QByteArray RootSyncDirUpdater::md5_summ(const QString & filepath) {
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) return QByteArray();

    hash.addData(&file);
    return hash.result();
}
