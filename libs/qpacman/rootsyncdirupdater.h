#ifndef ROOTSYNCDIRUPDATER_H
#define ROOTSYNCDIRUPDATER_H

#include <QEventLoop>
#include <QStringList>
#include <QDir>

class AlpmLockingNotifier;

class RootSyncDirUpdater : public QEventLoop {
    Q_OBJECT
public:
    RootSyncDirUpdater(const QString & sysSyncDir,const QString & userSyncDir,AlpmLockingNotifier * locking_notifier = NULL,QObject *parent = nullptr);
    QString errorString() const { return m_error; }
private slots:
    void process();
signals:
    void error(const QString & err);
    void locked_files(const QStringList & files);
    void need_alpm_reload();
    void ok();
private:
    void updateSystemSyncDir(const QDir & sys_sync_path,const QDir & user_sync_path);
    bool copyDirectoryFiles(const QString & fromDir,const QString & toDir,const QString & suffix = QString());
    static qint64 compare_file_dates(const QString & left,const QString & right);
    static int compare_file_sizes(const QString & left,const QString & right);
    static const QStringList leave_only_real_db_files(const QStringList & list);
    static bool isLeftDirNewer(const QDir & left,const QDir & right);
    static const QStringList dirContents(const QDir & dir,const QString & nameFilter);
    static int have_file_with_same_name(const QStringList & arr,const QString & name);
    static const QByteArray md5_summ(const QString & filepath);

    QString m_error;
    AlpmLockingNotifier * m_locking_notifier;
    QString m_sysSyncDir;
    QString m_userSyncDir;
};

#endif // ROOTSYNCDIRUPDATER_H
