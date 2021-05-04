/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef QPACMANSERVICE_H
#define QPACMANSERVICE_H

#include <QObject>
#include "libalpm.h"
#include "dbusstring.h"

class QPacmanService : public QObject {
    Q_OBJECT
public:
    explicit QPacmanService(QObject *parent = nullptr);

    Q_INVOKABLE bool queryPackages(const QString & name,int fieldType,int filter,const QString & group,const QString & repo);
    Q_INVOKABLE QString lastError();
    Q_INVOKABLE ThreadRun::RC installPackages(const String & root_pw,const QByteArray & pkgs,bool asdeps = false,const QByteArray & forcedpkgs = QByteArray());
    Q_INVOKABLE ThreadRun::RC processPackages(const String & root_pw);
    Q_INVOKABLE ThreadRun::RC downloadPackages(const QByteArray & pkgs);
    Q_INVOKABLE ThreadRun::RC updateDBs(bool force = false);
    Q_INVOKABLE QByteArray updates();
    Q_INVOKABLE void answer(uint value);
    Q_INVOKABLE QByteArray findLocalPackage(const QString & pkgname);
    Q_INVOKABLE QByteArray findLocalPackage(const QString & name,const QString & version);
    Q_INVOKABLE QByteArray findByPackageName(const QString & pkgname);
    Q_INVOKABLE QByteArray findByPackageNameProvides(const QByteArray & provide);
    Q_INVOKABLE QByteArray findLocalByPackageNameProvides(const QByteArray & provide);
    Q_INVOKABLE QString executingMethodName();
    Q_INVOKABLE void setMethodTerminateFlag();
    Q_INVOKABLE void askShowTrayOptions();
    Q_INVOKABLE bool removeLockFile();
    Q_INVOKABLE bool cleanCacheDirs();
    Q_INVOKABLE QStringList repos();
    Q_INVOKABLE QStringList groups();
    Q_INVOKABLE QStringList allDBs();
    Q_INVOKABLE QByteArray packageUrl(const QByteArray & pkg);
    Q_INVOKABLE QString packageFileName(const QByteArray & arr);
    Q_INVOKABLE qlonglong packageBuildDate(const QByteArray & arr);
    Q_INVOKABLE qlonglong packageInstallDate(const QByteArray & arr);
    Q_INVOKABLE QString packagePackager(const QByteArray & arr);
    Q_INVOKABLE QByteArray packageArch(const QByteArray & arr);
    Q_INVOKABLE QStringList packageLicenses(const QByteArray & arr);
    Q_INVOKABLE QStringList packageGroups(const QByteArray & arr);
    Q_INVOKABLE QByteArray packageDepends(const QByteArray & arr);
    Q_INVOKABLE QByteArray packageOptDepends(const QByteArray & arr);
    Q_INVOKABLE QByteArray packageConflicts(const QByteArray & arr);
    Q_INVOKABLE QByteArray packageProvides(const QByteArray & arr);
    Q_INVOKABLE QByteArray packageReplaces(const QByteArray & arr);
    Q_INVOKABLE int packageType(const QByteArray & arr);
    Q_INVOKABLE int packageChangeStatus(const QByteArray & arr);
    Q_INVOKABLE qlonglong packageSize(const QByteArray & arr);
    Q_INVOKABLE qlonglong packageInstalledSize(const QByteArray & arr);
    Q_INVOKABLE int packageReason(const QByteArray & arr);
    Q_INVOKABLE bool packageSetReason(const QByteArray & arr,int reason);
    Q_INVOKABLE bool packageIsOrphaned(const QByteArray & arr);
    Q_INVOKABLE bool packageSetChangeStatus(const QByteArray & arr,int status);
    Q_INVOKABLE QByteArray packagePossibleChangeStatuses(const QByteArray & arr);
    Q_INVOKABLE int packageDefaultStatus(const QByteArray & arr);
    Q_INVOKABLE QByteArray packageIconUrl(const QByteArray & arr);
    Q_INVOKABLE ThreadRun::RC packageFiles(const QByteArray & arr);
    Q_INVOKABLE uint downloaderTimeout();
    Q_INVOKABLE uint downloaderThreadCount();
    Q_INVOKABLE QByteArray downloaderProxy();
    Q_INVOKABLE bool useSystemIcons();
    Q_INVOKABLE void setDownloaderTimeout(uint value);
    Q_INVOKABLE void setDownloaderThreads(uint value);
    Q_INVOKABLE void setDownloaderProxy(const QByteArray & proxy);
    Q_INVOKABLE void setUsingSystemIcons(bool flag);
    Q_INVOKABLE bool areMarkedPackages();
    Q_INVOKABLE QString dbExtension();
    Q_INVOKABLE QStringList dbExtensions();
    Q_INVOKABLE bool setDBExtension(const QString & dbext);
    Q_INVOKABLE QString rootDir();
    Q_INVOKABLE QString dbPath();
    Q_INVOKABLE QString gpgDir();
    Q_INVOKABLE QString logFileName();
    Q_INVOKABLE QString arch();
    Q_INVOKABLE bool doUseSysLog();
    Q_INVOKABLE bool doDisableDownloadTimeout();
    Q_INVOKABLE QStringList sigLevel();
    Q_INVOKABLE QStringList localFileSigLevel();
    Q_INVOKABLE QStringList remoteFileSigLevel();
    Q_INVOKABLE QStringList holdPkgs();
    Q_INVOKABLE QStringList cacheDirs();
    Q_INVOKABLE QStringList syncFirstPkgs();
    Q_INVOKABLE QStringList hookDirs();
    Q_INVOKABLE QStringList ignoreGroups();
    Q_INVOKABLE QStringList ignorePkgs();
    Q_INVOKABLE QStringList noExtractPkgs();
    Q_INVOKABLE QStringList noUpgradePkgs();
    Q_INVOKABLE bool packageIsDownloaded(const QByteArray & pkg);
    Q_INVOKABLE QStringList packageRequiredby(const QByteArray & pkg);
    Q_INVOKABLE QStringList packageOptionalfor(const QByteArray & pkg);
    Q_INVOKABLE void resetPackageChangeStatuses();
    Q_INVOKABLE QByteArray createLocalPackage(const QString & pkgpath);
    Q_INVOKABLE void deleteLocalPackage(const QByteArray & pkg);

    Q_INVOKABLE void dbRefresherIsAboutToStart();
    Q_INVOKABLE void updaterAboutToStart();

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
    bool check_root_password(const QString & root_pw);
    static int pam_auth(int num_msg, const struct pam_message **msg,struct pam_response **resp, void *appdata_ptr);

    static bool m_files_executing;
    bool locked_outside;
    bool reload_is_needed;
    QString tempFileName;

    friend class FilesMethodPauser;
    friend class ActionApplier;
};

#endif // QPACMANSERVICE_H
