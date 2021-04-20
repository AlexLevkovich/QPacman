/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef LIBALPM_H
#define LIBALPM_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QList>
#include <QFlags>
#include <QDir>
#include <alpm.h>
#include "alpmdb.h"
#include "inotifier.h"
#include "alpmpackage.h"
#include "alpmconfig.h"
#include "alpmfuture.h"
#include "qalpmtypes.h"

class QFlagEventLoop;

class Alpm : public ThreadRun {
    Q_OBJECT
public:
    enum Caps {
        NLS = ALPM_CAPABILITY_NLS,
        DOWNLOADER = ALPM_CAPABILITY_DOWNLOADER,
        SIGNATURES = ALPM_CAPABILITY_SIGNATURES
    };
    
    enum Errors {
        PKG_LIST_IS_EMPTY             = -1,
        ALPM_INSTANCE_ALREADY_CREATED = -2,
        ALPM_IS_NOT_OPEN              = -3,
        ALPMDB_HANDLE_FAILED          = -4,
        LOCAL_DB_UPDATE               = -5,
        PKG_IS_NOT_INITED             = -6,
        REASON_WRONG_DB               = -7,
        CANNOT_GET_ROOT               = -9,
        ALPM_CONFIG_FAILED            = -10,
        ALPM_LINK_LOCAL_DB_FAILED     = -11,
        ALPM_HANDLE_FAILED            = -12,
        THREAD_IS_ALREADY_RUNNING     = -13,
        CANNOT_LOAD_CONFIG            = -14,
        USER_REFUSAL                  = -15,
        NOTHING_DOWNLOAD              = -16,
        OK_CODE_SUPPRESS_ALPMS        = -17,
        OK_CODE                       = ALPM_ERR_OK,
        OUT_OF_MEMORY                 = ALPM_ERR_MEMORY,
        UNEXPECTED_ERR                = ALPM_ERR_SYSTEM,
        BAD_PERMS                     = ALPM_ERR_BADPERMS,
        NOT_A_FILE                    = ALPM_ERR_NOT_A_FILE,
        NOT_A_DIR                     = ALPM_ERR_NOT_A_DIR,
        WRONG_ARGS                    = ALPM_ERR_WRONG_ARGS,
        NO_FREE_DISK_SPACE            = ALPM_ERR_DISK_SPACE,
        LIB_IS_NOT_INITIALIZED        = ALPM_ERR_HANDLE_NULL,
        LIB_IS_OPEN_ALREADY           = ALPM_ERR_HANDLE_NOT_NULL,
        UNABLE_TO_LOCK_DB             = ALPM_ERR_HANDLE_LOCK,
        CANNOT_OPEN_DB                = ALPM_ERR_DB_OPEN,
        CANNOT_CREATE_DB              = ALPM_ERR_DB_CREATE,
        DB_IS_NOT_INITIALIZED         = ALPM_ERR_DB_NULL,
        DB_IS_OPEN_ALREADY            = ALPM_ERR_DB_NOT_NULL,
        DB_NOT_FOUND                  = ALPM_ERR_DB_NOT_FOUND,
        DB_INVALID                    = ALPM_ERR_DB_INVALID,
        DB_INVALID_SIG                = ALPM_ERR_DB_INVALID_SIG,
        DB_INCORRECT_VERSION          = ALPM_ERR_DB_VERSION,
        CANNOT_UPDATE_DB              = ALPM_ERR_DB_WRITE,
        CANNOT_REMOVE_DB              = ALPM_ERR_DB_REMOVE,
        SERVER_BAD_URL                = ALPM_ERR_SERVER_BAD_URL,
        NO_CONFIGURED_SERVERS         = ALPM_ERR_SERVER_NONE,
        TRANS_IS_INITIALIZED_ALREADY  = ALPM_ERR_TRANS_NOT_NULL,
        TRANS_IS_NOT_INITIALIZED      = ALPM_ERR_TRANS_NOT_INITIALIZED,
        TRANS_IS_NOT_INITIALIZED_NULL = ALPM_ERR_TRANS_NULL,
        TRANS_DUP_TARGET              = ALPM_ERR_TRANS_DUP_TARGET,
        TRANS_IS_NOT_PREPARED         = ALPM_ERR_TRANS_NOT_PREPARED,
        TRANS_IS_ABORTED              = ALPM_ERR_TRANS_ABORT,
        TRANS_TYPE_UNCOMPATIBLE_OPER  = ALPM_ERR_TRANS_TYPE,
        TRANS_IS_NOT_LOCKED           = ALPM_ERR_TRANS_NOT_LOCKED,
        TRANS_HOOK_IS_FAILED          = ALPM_ERR_TRANS_HOOK_FAILED,
        PKG_IS_NOT_FOUND              = ALPM_ERR_PKG_NOT_FOUND,
        PKG_IS_IGNORED                = ALPM_ERR_PKG_IGNORED,
        PKG_IS_INVALID                = ALPM_ERR_PKG_INVALID,
        PKG_HAS_INVALID_CHECKSUM      = ALPM_ERR_PKG_INVALID_CHECKSUM,
        PKG_HAS_INVALID_SIG           = ALPM_ERR_PKG_INVALID_SIG,
        PKG_HAS_MISSING_SIG           = ALPM_ERR_PKG_MISSING_SIG,
        CANNOT_OPEN_PKG               = ALPM_ERR_PKG_OPEN,
        CANNOT_REMOVE_PKG             = ALPM_ERR_PKG_CANT_REMOVE,
        PKG_HAS_INVALID_NAME          = ALPM_ERR_PKG_INVALID_NAME,
        PKG_HAS_INVALID_ARCH          = ALPM_ERR_PKG_INVALID_ARCH,
        PKG_REPO_IS_NOT_FOUND         = ALPM_ERR_PKG_REPO_NOT_FOUND,
        SIG_IS_MISSING                = ALPM_ERR_SIG_MISSING,
        SIG_IS_INVALID                = ALPM_ERR_SIG_INVALID,
        UNSATISFIED_DEPS              = ALPM_ERR_UNSATISFIED_DEPS,
        CONFLICTING_DEPS              = ALPM_ERR_CONFLICTING_DEPS,
        FILE_CONFLICTS_ARE_FOUND      = ALPM_ERR_FILE_CONFLICTS,
        FAILED_RETRIEVE_FILES         = ALPM_ERR_RETRIEVE,
        INVALID_REGEX                 = ALPM_ERR_INVALID_REGEX,
        LIBARCHIVE_ERR                = ALPM_ERR_LIBARCHIVE,
        EXTERNAL_DOWNLOAD_ERR         = ALPM_ERR_EXTERNAL_DOWNLOAD,
        GPGME_ERR                     = ALPM_ERR_GPGME,
        MISSING_CAPABILITY_SIGNATURES = ALPM_ERR_MISSING_CAPABILITY_SIGNATURES
    };

    Q_DECLARE_FLAGS(CapsFlags,Caps)

    Alpm(QObject *parent = NULL);
    ~Alpm();
    bool open(const QString & confpath,const QString & dbpath = QString());
    bool reopen();
    bool close();
    bool isValid(bool change_errno = false) const;
    QString arch() const;
    QString lastError(int * error_id = NULL) const;
    ThreadRun::RC lastMethodRC() const;
    QString dbDirPath() const;
    QString lockFilePath() const;
    static const QString version();
    static CapsFlags capabilities();
    bool isLocked() const;
    bool removeLockFile();
    QStringList cacheDirs() const;
    bool cleanCacheDirs();

    QStringList repos() const;
    // you need to execute queryPackages() first to have groups() filled
    QStringList groups() const;
    QList<AlpmDB> allSyncDBs() const;
    AlpmDB localDB() const;
    QList<AlpmPackage> find(const QRegularExpression & expr) const;
    QList<AlpmPackage> findByPackageName(const QString & pkgname) const;
    QList<AlpmPackage> findByPackageNameVersion(const QString & pkgname,const QString & version) const;
    QList<AlpmPackage> findByPackageNameProvides(const AlpmPackage::Dependence & provide) const;
    //queryPackages: gets QList<AlpmPackage> result from methodFinished(func_name,QList<AlpmPackage> result,rc) signal
    bool queryPackages(const QString & name = QString(),AlpmPackage::SearchFieldType fieldType = AlpmPackage::NAME,AlpmPackage::PackageFilter filter = AlpmPackage::IS_ALL,const QString & group = QString(),const QString & repo = QString());
    //updateDBs: start updating sync dbs. It ends at methodFinished(func_name,rc) signal execution.
    ThreadRun::RC updateDBs(bool force = false);
    //installPackages: start installing packages. It ends at methodFinished(func_name,rc) signal execution.
    ThreadRun::RC installPackages(const QList<AlpmPackage> & pkgs,bool asdeps = false,const QList<AlpmPackage> & forcedpkgs = QList<AlpmPackage>());
    //removePackages: start removing packages. It ends at methodFinished(func_name,rc) signal execution.
    ThreadRun::RC removePackages(const QList<AlpmPackage> & pkgs,bool cascade = true);
    //downloadPackages: gets QStringList result from methodFinished(func_name,QStringList result,rc) signal
    ThreadRun::RC downloadPackages(const QList<AlpmPackage> & pkgs);
    QList<AlpmPackage> updates() const;

    void answer(bool flag);
    void answer(uint value);

    static Alpm * instance();
    static AlpmConfig * config();

    static bool isOpen();

signals:
    void method_finished(const QString & name,const QList<AlpmPackage> & result,ThreadRun::RC rc);
    void method_finished(const QString & name,const QStringList & result,ThreadRun::RC rc);

    // progresses
    void install_progress(const QString & pkg_name,int percent,int n_targets,int current_target);
    void remove_progress(const QString & pkg_name,int percent,int n_targets,int current_target);
    void conflicts_progress(int percent);
    void diskspace_progress(int percent);
    void integrity_progress(int percent);
    void load_progress(int percent);
    void keyring_progress(int percent);

    // downloads
    void downloads_starting();
    void full_download_size_found(qint64 total);
    void downloads_completed();
    void downloads_failed();
    void download_start(const QString & filename);
    void download_db_start(const QString & dbname);
    void download_done(const QString & filename);
    void download_failed(const QString & filename);
    void download_progress(const QString & filename,qint64 xfered,qint64 total);

    // questions. Qt::QueuedConnection is forbidden!!!
    void question(const QString & str);
    void install_packages_confirmation(const QStringList & install,const QStringList & remove,qint64 dl_size,qint64 install_size,qint64 remove_size);
    void remove_packages_confirmation(const QStringList & remove,qint64 remove_size);
    void select_provider(const QString & pkgname,const QStringList & providers);

    // informational events
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

    // events for possible database changes
    void locking_changed(const QString & path,bool locked);
    void pkgs_installed(const QStringList & installed_pkgs,const QStringList & removed_pkgs);
    void pkgs_removed(const QStringList & pkgs);

private slots:
    void operation_download_fn(const QString & filename,qint64 bytes_downloaded,qint64 length,int percents,qint64 speed);
    void lockFileChanged(const QString & path);
    void stop_waiting(int flag);
    void on_method_finished(const QString & name,const QVariant & result,ThreadRun::RC rc);

private:
    int wait_for_answer();
    void query_packages_portion(QList<AlpmPackage> & pkgs,int startindex,int lastindex) const;
    void sync_sysupgrade_portion(QList<AlpmPackage> & add_pkgs,int startindex,int lastindex);
    int sync_sysupgrade(int m_install_flags);
    void install_packages(const QList<AlpmPackage> & m_pkgs,int m_install_flags,const QList<AlpmPackage> & forcedpkgs);
    void remove_packages(const QList<AlpmPackage> & m_pkgs,bool remove_cascade);
    void update_dbs(bool force);
    QList<AlpmPackage> query_packages(const QString & name = QString(),AlpmPackage::SearchFieldType fieldType = AlpmPackage::NAME,AlpmPackage::PackageFilter filter = AlpmPackage::IS_ALL,const QString & group = QString(),const QString & repo = QString()) const;
    QStringList download_packages(const QList<AlpmPackage> & pkgs);
    QString download_package(const QString & download_url) const;
    QString download_package(const AlpmPackage & pkg) const;
    QList<AlpmPackage> check_updates() const;

    static const QStringList dirContents(const QDir & dir,const QString & nameFilter);
    static int operation_fetch_fn(const QString & url,const QString & localpath,bool force);
    static int operation_fetch_fn(const char *url, const char *localpath,int force);
    static void operation_progress_fn(alpm_progress_t progress, const char * pkg_name, int percent, size_t n_targets, size_t current_target);
    static void operation_question_fn(alpm_question_t * question);
    static void operation_event_fn(alpm_event_t * event);
    static const QString alpm_item_string_fn(alpm_pkg_t * value);
    static const QString dep_item_add(alpm_depend_t * value,StringStringMap & installed_deps,StringStringMap & pending_deps);
    static void display_optdepends(alpm_pkg_t * pkg);
    static void display_new_optdepends(alpm_pkg_t *oldpkg, alpm_pkg_t *newpkg);
    static int depend_cmp(const void *d1, const void *d2);
    static bool pkg_equal_cmp(const AlpmPackage & pkg1, const AlpmPackage & pkg2);
    static bool pkg_less_cmp(const AlpmPackage & pkg1, const AlpmPackage & pkg2);
    static bool updates_cmp(const AlpmPackage & item1, const AlpmPackage & item2);
    static bool sort_cmp(const AlpmPackage & item1, const AlpmPackage & item2);
    static bool string_name_cmp(const QString & item1,const QString & item2);
    static bool sort_equal_cmp(const AlpmPackage & item1, const AlpmPackage & item2);
    static int fnmatch_cmp(const void *pattern, const void *string);
    static bool pkgLessThan(const AlpmPackage & pkg1, const AlpmPackage & pkg2);
    static bool only_pkg_name_cmp(const AlpmPackage & item1, const AlpmPackage & item2);

    void emit_question(const QString & message,bool * answer);
    void emit_information(const QString & message,bool significant = false);
    void emit_error(const QString & message);
    void emit_install_packages_confirmation(const QStringList & install,const QStringList & remove,qint64 dl_size,qint64 install_size,qint64 remove_size,bool * answer);
    void emit_remove_packages_confirmation(const QStringList & remove,qint64 remove_size,bool * answer);
    void emit_progress(const char * signal,const QString & pkg_name,int percent,int n_targets,int current_target);
    void emit_progress(const char * signal,int percent);
    void emit_select_provider(const QString & pkgname,const QStringList & providers,int * answer);
    void emit_optdepends_event(const QString & pkgname,const StringStringMap & installed_deps,const StringStringMap & pending_deps);
    bool emit_event(const char *member,QGenericArgument val0 = QGenericArgument(),QGenericArgument val1 = QGenericArgument(),QGenericArgument val2 = QGenericArgument(),QGenericArgument val3 = QGenericArgument(),QGenericArgument val4 = QGenericArgument(),QGenericArgument val5 = QGenericArgument(),QGenericArgument val6 = QGenericArgument(),QGenericArgument val7 = QGenericArgument(),QGenericArgument val8 = QGenericArgument(),QGenericArgument val9 = QGenericArgument());

    bool do_process_targets(bool remove,QStringList & install_targets,QStringList & remove_targets);
    void recreatedbs();

    alpm_handle_t * m_alpm_handle;
    int m_alpm_errno;
    QStringList m_groups;
    Inotifier lock_watcher;
    AlpmConfig m_config;
    AlpmDB m_localDB;
    QFlagEventLoop * m_question_loop;
    QList<AlpmDB> m_syncDBs;
    QMap<AlpmPackage,QList<AlpmPackage> > m_replaces;

    static QStringList m_download_errs;
    static int prev_event_type;
    static int m_percent;
    static Alpm * p_alpm;
    static AlpmConfig * p_config;

    friend class AlpmPackage;
    friend class AlpmDB;
    friend class ThreadRun;
    friend class OverwriteHandler;
    friend class SearchPackage;
};

Q_DECLARE_METATYPE(StringStringMap)
Q_DECLARE_METATYPE(alpm_list_t *)
Q_DECLARE_METATYPE(size_t)
Q_DECLARE_OPERATORS_FOR_FLAGS(Alpm::CapsFlags)

#endif // LIBALPM_H
