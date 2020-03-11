/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef LIBALPM_H
#define LIBALPM_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QVector>
#include <QFlags>
#include <QDir>
#include <alpm.h>
#include "alpmdb.h"
#include "inotifier.h"
#include "alpmpackage.h"
#include "alpmconfig.h"
#include "alpmfuture.h"

typedef QMap<QString,QString > StringStringMap;

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
        LOCK_FILE_EXISTS              = -9,
        CANNOT_GET_ROOT               = -10,
        ALPM_CONFIG_FAILED            = -11,
        ALPM_LINK_LOCAL_DB_FAILED     = -12,
        ALPM_HANDLE_FAILED            = -13,
        THREAD_IS_ALREADY_RUNNING     = -14,
        CANNOT_LOAD_CONFIG            = -15,
        USER_REFUSAL                  = -16
    };

    Q_DECLARE_FLAGS(CapsFlags,Caps)

    Alpm(QObject *parent = NULL);
    ~Alpm();
    bool open(const QString & confpath,const QString & dbpath = QString());
    bool reopen();
    void close();
    bool isValid(bool change_errno = false) const;
    QString arch() const;
    QString lastError() const;
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
    QVector<AlpmPackage *> find(const QRegularExpression & expr) const;
    QVector<AlpmPackage *> findByFileName(const QString & filename) const;
    bool queryPackages(bool use_eventloop = true); //generates listing_packages_completed() signal if use_eventloop == true
    const QVector<AlpmPackage *> & lastQueriedPackages() const;
    ThreadRun::RC updateDBs(bool force = false);
    ThreadRun::RC installPackages(const QList<AlpmPackage *> & pkgs,bool asdeps = false,const QList<AlpmPackage *> & forcedpkgs = QList<AlpmPackage *>());
    ThreadRun::RC removePackages(const QList<AlpmPackage *> & pkgs,bool cascade = true);
    ThreadRun::RC downloadPackages(const QList<AlpmPackage *> & pkgs,QStringList * paths = NULL);
    QVector<AlpmPackage *> updates() const;


    static int pkg_vercmp(const QString & ver1, const QString & ver2);
    static Alpm * instance();
    static bool isOpen();

signals:
    void install_progress(const QString & pkg_name,int percent,size_t n_targets,size_t current_target);
    void remove_progress(const QString & pkg_name,int percent,size_t n_targets,size_t current_target);
    void conflicts_progress(int percent);
    void diskspace_progress(int percent);
    void integrity_progress(int percent);
    void load_progress(int percent);
    void keyring_progress(int percent);
    void download_starting();
    void full_download_size_found(qint64 total);
    void download_completed();
    void download_failed();
    void download_start(const QString & filename);
    void download_db_start(const QString & dbname);
    void download_done(const QString & filename);
    void download_failed(const QString & filename);
    void download_progress(const QString & filename,qint64 xfered,qint64 total);

    void question(const QString & str,bool * answer);
    void install_packages_confirmation(const QStringList & install,const QStringList & remove,qint64 dl_size,qint64 install_size,qint64 remove_size,bool * answer);
    void remove_packages_confirmation(const QStringList & remove,qint64 remove_size,bool * answer);
    void select_provider(const QString & pkgname,const QStringList & providers,int * answer);

    void optdepends_event(const QString & pkgname,const StringStringMap & installed_deps,const StringStringMap & pending_deps);
    void event(const QString & str);
    void error(const QString & str);
    void information(const QString & str,bool significant = false);
    void hook(const QString & str,int pos,int total);

    void listing_packages_completed();

    void pkgs_downloaded(const QStringList & paths);

    // event for possible database changes
    void locking_changed(const QString & path,bool locked);
    void dbs_updated(bool ok);
    void pkgs_installed(const QStringList & installed_pkgs,const QStringList & removed_pkgs);
    void pkgs_removed(const QStringList & pkgs);

private slots:
    void operation_download_fn(const QString & filename,qint64 bytes_downloaded,qint64 length,int percents,qint64 speed);
    void lockFileChanged(const QString & path);

private:
    void query_packages_portion(QVector<AlpmPackage *> & pkgs,int startindex,int lastindex);
    void sync_sysupgrade_portion(QVector<AlpmPackage *> & add_pkgs,int startindex,int lastindex);
    void fill_replaces();
    bool sync_sysupgrade();
    int install_packages(const QList<AlpmPackage *> & m_pkgs,int m_install_flags,const QList<AlpmPackage *> & forcedpkgs);
    int remove_packages(const QList<AlpmPackage *> & m_pkgs,bool remove_cascade);
    int update_dbs(bool force);
    int query_packages();
    QStringList download_packages(qint64 download_size,const QStringList & download_urls);
    QVector<AlpmPackage *> check_updates();

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
    static bool dup_repo_cmp(AlpmPackage * pkg1, AlpmPackage * pkg2);
    static bool updates_cmp(AlpmPackage * item1,AlpmPackage * item2);
    static bool sort_cmp(AlpmPackage * item1,AlpmPackage * item2);
    static bool string_name_cmp(const QString & item1,const QString & item2);
    static bool sort_equal_cmp(AlpmPackage * item1,AlpmPackage * item2);
    static int fnmatch_cmp(const void *pattern, const void *string);
    static bool pkgLessThan(AlpmPackage * pkg1, AlpmPackage * pkg2);
    static bool only_pkg_name_cmp(AlpmPackage * item1,AlpmPackage * item2);

    void emit_question(const QString & message,bool * answer);
    void emit_information(const QString & message,bool significant = false);
    void emit_event(const QString & message);
    void emit_error(const QString & message);
    void emit_install_packages_confirmation(const QStringList & install,const QStringList & remove,qint64 dl_size,qint64 install_size,qint64 remove_size,bool * answer);
    void emit_remove_packages_confirmation(const QStringList & remove,qint64 remove_size,bool * answer);
    void emit_progress(const char * signal,const QString & pkg_name,int percent,size_t n_targets,size_t current_target);
    void emit_progress(const char * signal,int percent);
    void emit_select_provider(const QString & pkgname,const QStringList & providers,int * answer);
    void emit_optdepends_event(const QString & pkgname,const StringStringMap & installed_deps,const StringStringMap & pending_deps);

    bool do_process_targets(bool remove,QStringList & install_targets,QStringList & remove_targets);
    int find_updates_in_list(const QVector<AlpmPackage *> & m_list,AlpmPackage * value);
    void recreatedbs();

    alpm_handle_t * m_alpm_handle;
    int m_alpm_errno;
    QString m_confpath;
    QString m_dbpath;
    QStringList m_groups;
    Inotifier lock_watcher;
    QString m_system_lock_file;
    AlpmConfig config;
    AlpmDB m_localDB;
    QList<AlpmDB> m_syncDBs;
    QVector<AlpmPackage *> m_packages;
    QMap<AlpmPackage *,QVector<AlpmPackage *> > m_replaces;

    static int prev_event_type;
    static int m_percent;
    static Alpm * p_alpm;
    static AlpmConfig * p_config;

    friend class AlpmPackage;
    friend class AlpmDB;
    friend class ThreadRun;
    friend class OverwriteHandler;
};

Q_DECLARE_METATYPE(StringStringMap)
Q_DECLARE_METATYPE(AlpmPackage *)
Q_DECLARE_METATYPE(alpm_list_t *)
Q_DECLARE_METATYPE(size_t)
Q_DECLARE_OPERATORS_FOR_FLAGS(Alpm::CapsFlags)

#endif // LIBALPM_H
