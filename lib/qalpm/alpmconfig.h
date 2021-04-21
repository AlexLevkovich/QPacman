/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ALPMCONFIG_H
#define ALPMCONFIG_H

#include <QStringList>
#include <QNetworkProxy>
#include <QVariant>

class QSettings;
typedef struct __alpm_handle_t alpm_handle_t;
typedef struct __alpm_list_t  alpm_list_t;

class AlpmConfig {
public:
    class Repo {
    public:
      bool isValid();
      QString name() const { return this->m_name; }
      QString arch() const { return this->m_arch; }
      QStringList servers() const { return this->m_servers; }
      int siglevel() const { return this->m_siglevel; }
      int usage() const { return m_usage; }

    private:
      Repo(const QString & name,const QString & arch,QSettings * settings,const QStringList & def_siglevel);
      bool setServersFromFile(const QString & filepath,QString & error);
      static bool config_parse_siglevel(const QStringList & values,int & level,QString & error);

      bool m_valid;
      friend class AlpmConfig;

      int m_usage;
      QString m_name;
      QString m_arch;
      QStringList m_servers;
      int m_siglevel;
    };

    AlpmConfig(const QString & conf_filepath = QString());
    ~AlpmConfig();
    bool setConfPath(const QString & conf_filepath);
    QString confPath() const;
    alpm_handle_t * translate(const QString & dbpath = QString());
    QString lastError() const;

    QString rootDir() const { return rootdir; }
    QString dbPath() const { return dbpath; }
    QString gpgDir() const { return gpgdir; }
    QString logFileName() const { return logfile; }
    QString arch() const { return architecture; }
    bool doUseSysLog() const { return usesyslog; }
    bool doDisableDownloadTimeout() const { return disabledownloadtimeout; }
    QStringList sigLevel() const { return siglevel; }
    QStringList localFileSigLevel() const { return localfilesiglevel; }
    QStringList remoteFileSigLevel() const { return remotefilesiglevel; }
    QStringList holdPkgs() const { return holdpkgs2; }
    QStringList cacheDirs() const { return cachedirs; }
    QStringList syncFirstPkgs() const { return syncfirst; }
    QStringList hookDirs() const { return hookdirs; }
    QStringList ignoreGroups() const { return ignoregroups; }
    QStringList ignorePkgs() const { return ignorepkgs; }
    QStringList noExtractPkgs() const { return noextract; }
    QStringList noUpgradePkgs() const { return noupgrade; }
    QList<Repo> repositories() const { return repos; }

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
    static const QString userConfFile();
    bool config_parse_siglevel(const QStringList & val,int & level);
    alpm_list_t * convert_list(const QStringList & list);

    QString m_error;
    QString conf_filepath;
    alpm_list_t * holdpkgs;

    QString rootdir;
    QString dbpath;
    QString gpgdir;
    QString logfile;
    QString architecture;
    bool usesyslog;
    bool disabledownloadtimeout;
    QStringList siglevel;
    QStringList localfilesiglevel;
    QStringList remotefilesiglevel;
    QStringList holdpkgs2;
    QStringList cachedirs;
    QStringList syncfirst;
    QStringList hookdirs;
    QStringList ignoregroups;
    QStringList ignorepkgs;
    QStringList noextract;
    QStringList noupgrade;
    QList<Repo> repos;

    friend class Alpm;
};

#endif // ALPMCONFIG_H
