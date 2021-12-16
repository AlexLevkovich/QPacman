/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef QPACMANSERVICE_H
#define QPACMANSERVICE_H

#include <QObject>
#include <QDBusContext>
#include "libalpm.h"
#include "dbusstring.h"

class QPacmanService : public QObject, public QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.alexl.qt.QPacmanService")
public:
    explicit QPacmanService(QObject *parent = nullptr);

public slots:
    bool queryPackages(const QString & name,int fieldType,int filter,const QString & group,const QString & repo);
    QString lastError();
    ThreadRun::RC installPackages(const String & root_pw,const QByteArray & pkgs,bool asdeps = false,const QByteArray & forcedpkgs = QByteArray());
    ThreadRun::RC processPackages(const String & root_pw);
    ThreadRun::RC downloadPackages(const QByteArray & pkgs);
    ThreadRun::RC updateDBs(bool force = false);
    QByteArray updates();
    void answer(uint value);
    QByteArray findLocalPackage(const QString & pkgname);
    QByteArray findLocalPackage(const QString & name,const QString & version);
    QByteArray findByPackageName(const QString & pkgname);
    QByteArray findByPackageNameProvides(const QByteArray & provide);
    QByteArray findLocalByPackageNameProvides(const QByteArray & provide);
    QString executingMethodName();
    void setMethodTerminateFlag();
    void askShowTrayOptions();
    bool removeLockFile();
    bool cleanCacheDirs();
    QByteArray repos();
    QStringList groups();
    QStringList allDBs();
    QByteArray packageUrl(const QByteArray & pkg);
    QString packageFileName(const QByteArray & arr);
    qlonglong packageBuildDate(const QByteArray & arr);
    qlonglong packageInstallDate(const QByteArray & arr);
    QString packagePackager(const QByteArray & arr);
    QByteArray packageArch(const QByteArray & arr);
    QStringList packageLicenses(const QByteArray & arr);
    QStringList packageGroups(const QByteArray & arr);
    QByteArray packageDepends(const QByteArray & arr);
    QByteArray packageOptDepends(const QByteArray & arr);
    QByteArray packageConflicts(const QByteArray & arr);
    QByteArray packageProvides(const QByteArray & arr);
    QByteArray packageReplaces(const QByteArray & arr);
    int packageType(const QByteArray & arr);
    int packageChangeStatus(const QByteArray & arr);
    qlonglong packageSize(const QByteArray & arr);
    qlonglong packageInstalledSize(const QByteArray & arr);
    int packageReason(const QByteArray & arr);
    bool packageSetReason(const QByteArray & arr,int reason);
    bool packageIsOrphaned(const QByteArray & arr);
    bool packageSetChangeStatus(const QByteArray & arr,int status);
    QByteArray packagePossibleChangeStatuses(const QByteArray & arr);
    int packageDefaultStatus(const QByteArray & arr);
    QByteArray packageIconUrl(const QByteArray & arr);
    ThreadRun::RC packageFiles(const QByteArray & arr);
    uint downloaderTimeout();
    uint downloaderThreadCount();
    QByteArray downloaderProxy();
    bool useSystemIcons();
    void setDownloaderTimeout(uint value);
    void setDownloaderThreads(uint value);
    void setDownloaderProxy(const QByteArray & proxy);
    void setUsingSystemIcons(bool flag);
    bool areMarkedPackages();
    QString dbExtension();
    QStringList dbExtensions();
    bool setDBExtension(const QString & dbext);
    QString rootDir();
    QString dbPath();
    QString gpgDir();
    QString logFileName();
    QStringList arches();
    bool doUseSysLog();
    bool doDisableDownloadTimeout();
    QStringList sigLevel();
    QStringList localFileSigLevel();
    QStringList remoteFileSigLevel();
    QStringList holdPkgs();
    QStringList cacheDirs();
    QStringList syncFirstPkgs();
    QStringList hookDirs();
    QStringList ignoreGroups();
    QStringList ignorePkgs();
    QStringList noExtractPkgs();
    QStringList noUpgradePkgs();
    bool packageIsDownloaded(const QByteArray & pkg);
    QStringList packageRequiredby(const QByteArray & pkg);
    QStringList packageOptionalfor(const QByteArray & pkg);
    void resetPackageChangeStatuses();
    QByteArray createLocalPackage(const QString & pkgpath);
    void deleteLocalPackage(const QByteArray & pkg);
    bool addNewRepo(const QByteArray & repo);
    bool addMirrorRepo(const QByteArray & repo);
    bool deleteRepo(const QString & name);
    void dbRefresherIsAboutToStart();
    void updaterAboutToStart();

signals:
    void show_tray_options();
    void file_queried(const QByteArray & file,int index,int count);

    void install_progress(const QString & pkg_name,int percent,int n_targets,int current_target);
    void remove_progress(const QString & pkg_name,int percent,int n_targets,int current_target);
    void conflicts_progress(int percent);
    void diskspace_progress(int percent);
    void integrity_progress(int percent);
    void load_progress(int percent);
    void keyring_progress(int percent);

    void downloads_starting();
    void full_download_size_found(qint64 total);
    void downloads_completed();
    void downloads_failed();
    void download_start(const QString & filename);
    void download_db_start(const QString & dbname);
    void download_done(const QString & filename);
    void download_failed(const QString & filename);
    void download_progress(const QString & filename,qint64 xfered,qint64 total);

    void question(const QString & str);
    void install_packages_confirmation(const QStringList & install,const QStringList & remove,qint64 dl_size,qint64 install_size,qint64 remove_size);
    void remove_packages_confirmation(const QStringList & remove,qint64 remove_size);
    void select_provider(const QString & pkgname,const QStringList & providers);

    void optdepends_event(const QString & pkgname,const StringStringMap & installed_deps,const StringStringMap & pending_deps);
    void error(const QString & str);
    void information(const QString & str,bool significant = false);
    void all_hooks(const QString & infostr);
    void hook(const QString & str,int pos,int total);
    void hook_completed(const QString & name);
    void all_hooks_completed();
    void checking_file_conflicts(const QString & infostr);
    void file_conflicts_checked();
    void checking_pkg_deps(const QString & infostr);
    void pkg_deps_checked();
    void checking_internal_conflicts(const QString & infostr);
    void internal_conflicts_checked();
    void transaction_completed();
    void checking_integrity(const QString & infostr);
    void integrity_checked();
    void checking_diskspace(const QString & infostr);
    void diskspace_checked();
    void resolving_pkg_deps(const QString & infostr);
    void pkg_deps_resolved();
    void checking_keyring(const QString & infostr);
    void keyring_checked();
    void checking_key_download(const QString & infostr);
    void key_download_checked();
    void loading_pkg_files(const QString & infostr);
    void pkg_files_loaded();
    void starting_scriplet(const QString & infostr);
    void scriplet_executed();

    void method_finished(const QString & name,ThreadRun::RC rc);
    void package_queried(const QByteArray & result);
    void method_finished(const QString & name,const QStringList & result,ThreadRun::RC rc);

    void alpm_reopen();
    void do_start_dbrefresher();
    void dbs_update_started();
    void do_start_package_updater();
    void package_updater_started();

private slots:
    void onmethod_finished(const QString & name,const QList<AlpmPackage> & result,ThreadRun::RC rc);
    void onmethod_finished(const QString & name,ThreadRun::RC rc);
    void onmethod_finished(const QString & name,const QStringList & result,ThreadRun::RC rc);
    void locking_changed(const QString & path,bool locked);

private:
    ThreadRun::RC install_packages(const QList<AlpmPackage> & pkgs,bool asdeps = false,const QList<AlpmPackage> & forcedpkgs = QList<AlpmPackage>());
    void remove_temp_file();
    void do_alpm_reopen();
    QString dbus_client_username() const;
    bool check_password(const QString & pw);
    static int pam_auth(int num_msg, const struct pam_message **msg,struct pam_response **resp, void *appdata_ptr);
    static const QString username_of_pid(pid_t pid);

    static bool m_files_executing;
    bool locked_outside;
    bool reload_is_needed;
    QString tempFileName;

    friend class FilesMethodPauser;
    friend class ActionApplier;
};

#endif // QPACMANSERVICE_H
