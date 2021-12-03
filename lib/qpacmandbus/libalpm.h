/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef LIBALPM_H
#define LIBALPM_H

#include <QObject>
#include <QStringList>
#include <QDateTime>
#include <QDBusPendingReply>
#include <QNetworkProxy>
#include "alpmpackage.h"
#include "qalpmtypes.h"

class ComAlexlQtQPacmanServiceInterface;
class QDBusServiceWatcher;

class Alpm : public QObject {
    Q_OBJECT
public:
    class Repo {
    public:

      enum SigObject {
          Package = (1 << 0),
          Database = (1 << 1),
          Both = (1 << 2)
      };

      enum SigCheck {
          Optional = (1 << 0),
          Required = (1 << 1),
          Never = (1 << 2),
          Default = (1 << 3)
      };

      enum SigAllowed {
          TrustedOnly = (1 << 0),
          TrustAll = (1 << 1),
          Nothing = (1 << 2)
      };

      class SigLevel {
      public:
          SigLevel();
          SigLevel(SigObject object,SigCheck check,SigAllowed allowed);

          bool operator==(const SigLevel & other) const;

          SigObject object() const { return m_object; }
          SigCheck check() const { return m_check; }
          SigAllowed allowed() const { return m_allowed; }

          friend QDataStream & operator<<(QDataStream &argument,const SigLevel & level);
          friend const QDataStream & operator>>(const QDataStream &argument,SigLevel & level);

       private:
          SigObject m_object;
          SigCheck m_check;
          SigAllowed m_allowed;
      };

      class ListSigLevel: public QList<SigLevel> {
      public:
          ListSigLevel() : QList<SigLevel>() {}
      };

      class Usage {
      public:
          Usage();
          Usage(bool sync,bool search,bool install,bool upgrade);

          bool isSync() const { return m_sync; }
          bool isSearch() const { return m_search; }
          bool isInstall() const { return m_install; }
          bool isUpgrade() const { return m_upgrade; }
          bool isAll() const;

          friend QDataStream & operator<<(QDataStream &argument,const Usage & usage);
          friend const QDataStream & operator>>(const QDataStream &argument,Usage & usage);
      private:
          bool m_sync;
          bool m_search;
          bool m_install;
          bool m_upgrade;
      };

      Repo();
      Repo(const Repo & repo);
      Repo(const QString & name,const QStringList & urls,const QStringList & arches,const ListSigLevel & siglevels,const Usage & usages);

      bool isValid() const;
      QString name() const { return this->m_name; }
      QStringList arches() const { return this->m_arches; }
      QStringList servers() const { return this->m_servers; }
      ListSigLevel siglevel() const { return this->m_siglevel; }
      Usage usage() const { return m_usage; }

      friend QDataStream & operator<<(QDataStream &argument,const Repo & repo);
      friend const QDataStream & operator>>(const QDataStream &argument,Repo & repo);
      Repo & operator=(const Repo &repo);

    private:
      bool m_valid;
      Usage m_usage;
      QString m_name;
      QStringList m_arches;
      QStringList m_servers;
      ListSigLevel m_siglevel;
    };

    Alpm(QObject * parent = NULL);
    ~Alpm();
    static Alpm * instance();
    QString lastError() const;
    bool isValid() const;
    bool queryPackages(const QString & name = QString(),AlpmPackage::SearchFieldType fieldType = AlpmPackage::NAME,AlpmPackage::PackageFilter filter = AlpmPackage::IS_ALL,const QString & group = QString(),const QString & repo = QString());
    ThreadRun::RC installPackages(const QString & root_pw,const QList<AlpmPackage> & pkgs,bool asdeps = false,const QList<AlpmPackage> & forcedpkgs = QList<AlpmPackage>());
    ThreadRun::RC processPackages(const QString & root_pw);
    ThreadRun::RC downloadPackages(const QList<AlpmPackage> & pkgs);
    ThreadRun::RC updateDBs(bool force = false);
    QList<AlpmPackage> updates() const;
    AlpmPackage findLocalPackage(const QString & pkgname) const;
    AlpmPackage findLocalPackage(const QString & name,const QString & version) const;
    QList<AlpmPackage> findByPackageName(const QString & pkgname) const;
    QList<AlpmPackage> findByPackageNameProvides(const AlpmPackage::Dependence & provide) const;
    QList<AlpmPackage> findLocalByPackageNameProvides(const AlpmPackage::Dependence & provide) const;
    bool answer(uint flag);
    QString executingMethodName() const;
    void setMethodTerminateFlag();
    bool askShowTrayOptions();
    bool removeLockFile();
    bool cleanCacheDirs();
    QList<Repo> repos() const;
    QStringList groups() const;
    QStringList allDBs() const;
    bool areMarkedPackages() const;

    bool addNewRepo(const Alpm::Repo & repo);
    bool addMirrorRepo(const Alpm::Repo & repo);
    bool deleteRepo(const QString & name);

    uint downloaderTimeout() const;
    uint downloaderThreadCount() const;
    QNetworkProxy downloaderProxy() const;
    bool useSystemIcons() const;
    void setDownloaderTimeout(uint value);
    void setDownloaderThreads(uint value);
    void setDownloaderProxy(const QNetworkProxy & proxy);
    void setUsingSystemIcons(bool flag);
    QString dbExtension() const;
    QStringList dbExtensions() const;
    bool setDBExtension(const QString & dbext);
    QString rootDir() const;
    QString dbPath() const;
    QString gpgDir() const;
    QString logFileName() const;
    QStringList arches() const;
    bool doUseSysLog() const;
    bool doDisableDownloadTimeout() const;
    QStringList sigLevel() const;
    QStringList localFileSigLevel() const;
    QStringList remoteFileSigLevel() const;
    QStringList holdPkgs() const;
    QStringList cacheDirs() const;
    QStringList syncFirstPkgs() const;
    QStringList hookDirs() const;
    QStringList ignoreGroups() const;
    QStringList ignorePkgs() const;
    QStringList noExtractPkgs() const;
    QStringList noUpgradePkgs() const;

    AlpmPackage createLocalPackage(const QString & pkgpath) const;
    void deleteLocalPackage(const AlpmPackage & pkg);

    void dbRefresherIsAboutToStart();
    void updaterAboutToStart();

signals:
    void all_hooks(const QString &infostr);
    void all_hooks_completed();
    void checking_diskspace(const QString &infostr);
    void checking_file_conflicts(const QString &infostr);
    void checking_integrity(const QString &infostr);
    void checking_internal_conflicts(const QString &infostr);
    void checking_key_download(const QString &infostr);
    void checking_keyring(const QString &infostr);
    void checking_pkg_deps(const QString &infostr);
    void conflicts_progress(int percent);
    void diskspace_checked();
    void diskspace_progress(int percent);
    void download_db_start(const QString &dbname);
    void download_done(const QString &filename);
    void download_failed(const QString &filename);
    void download_progress(const QString &filename, qlonglong xfered, qlonglong total);
    void download_start(const QString &filename);
    void downloads_completed();
    void downloads_failed();
    void downloads_starting();
    void error(const QString &str);
    void file_conflicts_checked();
    void full_download_size_found(qlonglong total);
    void hook(const QString &str, int pos, int total);
    void hook_completed(const QString &name);
    void information(const QString &str, bool significant);
    void install_packages_confirmation(const QStringList &install, const QStringList &remove, qlonglong dl_size, qlonglong install_size, qlonglong remove_size);
    void install_progress(const QString &pkg_name, int percent, int n_targets, int current_target);
    void integrity_checked();
    void integrity_progress(int percent);
    void internal_conflicts_checked();
    void key_download_checked();
    void keyring_checked();
    void keyring_progress(int percent);
    void load_progress(int percent);
    void loading_pkg_files(const QString &infostr);
    void method_finished(const QString &name, ThreadRun::RC rc);
    void package_queried(const AlpmPackage &pkg);
    void method_finished(const QString &name, const QStringList &result, ThreadRun::RC rc);
    void optdepends_event(const QString &pkgname, StringStringMap installed_deps, StringStringMap pending_deps);
    void pkg_deps_checked();
    void pkg_deps_resolved();
    void pkg_files_loaded();
    void question(const QString &str);
    void remove_packages_confirmation(const QStringList &remove, qlonglong remove_size);
    void remove_progress(const QString &pkg_name, int percent, int n_targets, int current_target);
    void resolving_pkg_deps(const QString &infostr);
    void scriplet_executed();
    void select_provider(const QString &pkgname, const QStringList &providers);
    void show_tray_options();
    void starting_scriplet(const QString &infostr);
    void transaction_completed();
    void alpm_reopen();
    void do_start_dbrefresher();
    void dbs_update_started();
    void do_start_package_updater();
    void package_updater_started();

private slots:
    void onpackage_queried(const QByteArray &result);
    void init();

private:
    QUrl packageUrl(const AlpmPackage & pkg) const;
    QString packageFileName(const AlpmPackage & pkg) const;
    QDateTime packageBuildDate(const AlpmPackage & pkg) const;
    QDateTime packageInstallDate(const AlpmPackage & pkg) const;
    QString packagePackager(const AlpmPackage & pkg) const;
    QByteArray packageArch(const AlpmPackage & pkg) const;
    QStringList packageLicenses(const AlpmPackage & pkg) const;
    QStringList packageGroups(const AlpmPackage & pkg) const;
    QList<AlpmPackage::Dependence> packageDepends(const AlpmPackage & pkg) const;
    QList<AlpmPackage::Dependence> packageOptDepends(const AlpmPackage & pkg) const;
    QList<AlpmPackage::Dependence> packageConflicts(const AlpmPackage & pkg) const;
    QList<AlpmPackage::Dependence> packageProvides(const AlpmPackage & pkg) const;
    QList<AlpmPackage::Dependence> packageReplaces(const AlpmPackage & pkg) const;
    AlpmPackage::Type packageType(const AlpmPackage & pkg) const;
    AlpmPackage::UserChangeStatus packageChangeStatus(const AlpmPackage & pkg) const;
    qint64 packageSize(const AlpmPackage & pkg) const;
    qint64 packageInstalledSize(const AlpmPackage & pkg) const;
    AlpmPackage::Reason packageReason(const AlpmPackage & pkg) const;
    bool packageSetReason(const AlpmPackage & pkg,AlpmPackage::Reason reason) const;
    bool packageIsOrphaned(const AlpmPackage & pkg) const;
    bool packageSetChangeStatus(const AlpmPackage & pkg,AlpmPackage::UserChangeStatus status) const;
    QList<AlpmPackage::UserChangeStatus> packagePossibleChangeStatuses(const AlpmPackage & pkg);
    AlpmPackage::UserChangeStatus packageDefaultStatus(const AlpmPackage & pkg) const;
    QUrl packageIconUrl(const AlpmPackage & pkg) const;
    QList<AlpmPackage::FileInfo> packageFiles(const AlpmPackage & pkg);
    bool packageIsDownloaded(const AlpmPackage & pkg) const;
    QStringList packageRequiredby(const AlpmPackage & pkg);
    QStringList packageOptionalfor(const AlpmPackage & pkg);
    void resetPackageChangeStatuses();

    template<typename T> bool replyToValue(const QDBusPendingReply<T> & in_reply,T & ret) const {
        Alpm * p_this = (Alpm *)this;
        p_this->m_error.clear();
        QDBusPendingReply<T> & reply = (QDBusPendingReply<T> &)in_reply;
        reply.waitForFinished();
        if (reply.isError()) {
            p_this->m_error = reply.error().message();
            emit p_this->error(m_error);
            return false;
        }
        ret = reply.value();
        return true;
    }

    bool replyToVoid(const QDBusPendingReply<> & in_reply) const {
        Alpm * p_this = (Alpm *)this;
        p_this->m_error.clear();
        QDBusPendingReply<> & reply = (QDBusPendingReply<> &)in_reply;
        reply.waitForFinished();
        if (reply.isError()) {
            p_this->m_error = reply.error().message();
            emit p_this->error(m_error);
            return false;
        }
        return true;
    }

    QDBusServiceWatcher * m_watcher;
    ComAlexlQtQPacmanServiceInterface * m_interface;
    QString m_error;
    bool m_valid;
    friend class AlpmPackage;
};

#endif // LIBALPM_H
