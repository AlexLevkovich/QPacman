/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ALPMCONFIG_H
#define ALPMCONFIG_H

#include <QStringList>
#include <QList>
#include <QNetworkProxy>
#include <QVariant>

class ConfReader;
typedef struct __alpm_handle_t alpm_handle_t;
typedef struct __alpm_list_t  alpm_list_t;

class AlpmConfig {
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

      class ListSigLevel;

      class SigLevel {
      public:
          SigLevel();
          SigLevel(SigObject object,SigCheck check,SigAllowed allowed);

          QString toString() const;

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

          friend class ListSigLevel;
      };

      class ListSigLevel: public QList<SigLevel> {
      public:
          ListSigLevel() : QList<SigLevel>() {}
          ListSigLevel(const QStringList & values);
          QString toString() const;
          QStringList toStringList() const;
          operator uint() const;

      private:
          ListSigLevel(uint level);
          void siglevel_to_int(uint & ret,bool is_package,SigCheck check,SigAllowed allowed) const;

          friend class AlpmConfig;
      };

      class Usage {
      public:
          Usage();
          Usage(const QStringList & values);
          QString toString() const;

          bool isSync() const { return m_sync; }
          bool isSearch() const { return m_search; }
          bool isInstall() const { return m_install; }
          bool isUpgrade() const { return m_upgrade; }
          bool isAll() const;
          operator uint() const;

          friend QDataStream & operator<<(QDataStream &argument,const Usage & usage);
          friend const QDataStream & operator>>(const QDataStream &argument,Usage & usage);
      private:
          Usage(uint val);

          bool m_sync;
          bool m_search;
          bool m_install;
          bool m_upgrade;

          friend class AlpmConfig;
      };

      Repo(const QString & name,const QStringList & urls,const QStringList & arches,const ListSigLevel & siglevels,const Usage & usages);
      Repo(const Repo & repo);
      Repo();
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
      Repo(const QString & name,const QStringList & arches,ConfReader * settings);
      bool addServersFromFile(const QString & filepath,QString & error);

      QString m_orig_name;
      bool m_valid;
      friend class AlpmConfig;

      Usage m_usage;
      QString m_name;
      QStringList m_arches;
      QStringList m_servers;
      ListSigLevel m_siglevel;
    };

    ~AlpmConfig();
    bool setConfPath(const QString & conf_filepath);
    QString confPath() const;
    QString lastError() const;

    QString rootDir() const { return rootdir; }
    QString dbPath() const { return dbpath; }
    QString gpgDir() const { return gpgdir; }
    QString logFileName() const { return logfile; }
    QStringList arches() const { return architectures; }
    bool doUseSysLog() const { return usesyslog; }
    bool doDisableDownloadTimeout() const { return disabledownloadtimeout; }
    Repo::ListSigLevel sigLevel() const { return siglevel; }
    Repo::ListSigLevel localFileSigLevel() const { return localfilesiglevel; }
    Repo::ListSigLevel remoteFileSigLevel() const { return remotefilesiglevel; }
    QStringList holdPkgs() const { return holdpkgs2; }
    QStringList cacheDirs() const { return cachedirs; }
    QStringList syncFirstPkgs() const { return syncfirst; }
    QStringList hookDirs() const { return hookdirs; }
    QStringList ignoreGroups() const { return ignoregroups; }
    QStringList ignorePkgs() const { return ignorepkgs; }
    QStringList noExtractPkgs() const { return noextract; }
    QStringList noUpgradePkgs() const { return noupgrade; }
    QList<Repo> repositories() const { return repos; }

    bool addNewRepo(const Repo & repo);
    bool addMirrorRepo(const Repo & repo);
    bool deleteRepo(const QString & name);

    static const QString dbExtension();
    static const QStringList dbExtensions();
    static uint downloaderTimeout();
    static uint downloaderThreadCount();
    static const QNetworkProxy downloaderProxy();
    static bool useSystemIcons();
    static bool setDBExtension(const QString & dbext);
    static void setDownloaderTimeout(uint value);
    static void setDownloaderThreads(uint value);
    static void setDownloaderProxy(const QNetworkProxy & proxy);
    static void setUsingSystemIcons(bool flag);
private:
    AlpmConfig();
    alpm_handle_t * translate(const QString & dbpath = QString());

    static const QString user_name();
    static const QString user_dir();
    static const QString user_conf_file();
    static const QStringList split_options(const QStringList & list);
    alpm_list_t * convert_list(const QStringList & list);
    int index_of_repo(const QString & name);
    static bool test_flag(uint level,uint flag);

    QString m_error;
    QString conf_filepath;
    alpm_list_t * holdpkgs;

    QString rootdir;
    QString dbpath;
    QString gpgdir;
    QString logfile;
    QStringList architectures;
    bool usesyslog;
    bool disabledownloadtimeout;
    Repo::ListSigLevel siglevel;
    Repo::ListSigLevel localfilesiglevel;
    Repo::ListSigLevel remotefilesiglevel;
    QStringList holdpkgs2;
    QStringList cachedirs;
    QStringList syncfirst;
    QStringList hookdirs;
    QStringList ignoregroups;
    QStringList ignorepkgs;
    QStringList noextract;
    QStringList noupgrade;
    QList<Repo> repos;
    alpm_handle_t * m_alpm_handle;

    friend class Alpm;
};

#endif // ALPMCONFIG_H
