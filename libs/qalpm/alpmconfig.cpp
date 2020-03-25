/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "alpmconfig.h"
#include <sys/utsname.h>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDataStream>
#include <sys/sysinfo.h>

AlpmConfig::AlpmConfig(const QString & conf_filepath) {
    holdpkgs = NULL;
    setConfPath(conf_filepath);
}

AlpmConfig::~AlpmConfig() {
    FREELIST(holdpkgs);
}

bool AlpmConfig::config_parse_siglevel(const QStringList & val,int & level) {
    return AlpmConfig::Repo::config_parse_siglevel(val,level,m_error);
}

alpm_list_t * AlpmConfig::convert_list(const QStringList & list) {
    alpm_list_t * ret = NULL;

    for (int i=0;i<list.count();i++) {
        ret = alpm_list_add(ret,strdup(list[i].toLocal8Bit().constData()));
    }

    return ret;
}

bool AlpmConfig::setConfPath(const QString & conf_filepath) {
    this->conf_filepath.clear();
    rootdir.clear();
    dbpath.clear();
    gpgdir.clear();
    logfile.clear();
    architecture.clear();
    usesyslog = 0;
    disabledownloadtimeout = 0;
    siglevel.clear();
    localfilesiglevel.clear();
    remotefilesiglevel.clear();
    holdpkgs2.clear();
    cachedirs.clear();
    hookdirs.clear();
    ignoregroups.clear();
    ignorepkgs.clear();
    noextract.clear();
    noupgrade.clear();
    repos.clear();

    m_error.clear();
    QSettings ini_file(conf_filepath.isEmpty()?PACMANCONF:conf_filepath,QSettings::IniFormat);
    QStringList groups = ini_file.childGroups();
    if (!groups.contains("options")) {
        m_error = QObject::tr("Can't find [options] section!");
        return false;
    }

    rootdir = ini_file.value("options/RootDir","/").toString();
    dbpath = ini_file.value("options/DBPath",DBPATH).toString();
    gpgdir = ini_file.value("options/GPGDir",GPGDIR).toString();
    logfile = ini_file.value("options/LogFile",LOGFILE).toString();
    architecture = ini_file.value("options/Architecture","auto").toString();
    if (architecture == "auto") {
        struct utsname un;
        if(uname(&un) != 0) {
            m_error = QObject::tr("Can't determine system's architecture!");
            return false;
        }
        architecture = un.machine;
    }
    usesyslog = ini_file.value("options/UseSyslog",1).toInt();
    disabledownloadtimeout = (bool)ini_file.value("options/DisableDownloadTimeout",0).toInt();
    siglevel = ini_file.value("options/SigLevel","Never").toString().trimmed().split(" ",QString::SkipEmptyParts);
    localfilesiglevel = ini_file.value("options/LocalFileSigLevel",siglevel.join(" ")).toString().trimmed().split(" ",QString::SkipEmptyParts);
    remotefilesiglevel = ini_file.value("options/RemoteFileSigLevel",siglevel.join(" ")).toString().trimmed().split(" ",QString::SkipEmptyParts);
    holdpkgs2 = ini_file.value("options/HoldPkg","").toString().split(" ",QString::SkipEmptyParts);
    cachedirs = ini_file.value("options/CacheDir",CACHEDIR).toString().split(" ",QString::SkipEmptyParts);
    if (getuid() != 0) cachedirs.prepend(QDir::tempPath());
    hookdirs = ini_file.value("options/HookDir",HOOKDIR).toString().split(" ",QString::SkipEmptyParts);
    ignoregroups = ini_file.value("options/IgnoreGroup","").toString().split(" ",QString::SkipEmptyParts);
    ignorepkgs = ini_file.value("options/IgnorePkg","").toString().split(" ",QString::SkipEmptyParts);
    noextract = ini_file.value("options/NoExtract","").toString().split(" ",QString::SkipEmptyParts);
    noupgrade = ini_file.value("options/NoUpgrade","").toString().split(" ",QString::SkipEmptyParts);

    for (int i=0;i<groups.count();i++) {
        if (groups[i] == "options") continue;
        Repo repo(groups[i],architecture,&ini_file,siglevel);
        if (!repo.isValid()) continue;
        repos.append(repo);
    }
    this->conf_filepath = ini_file.fileName();

    return true;
}

const QString AlpmConfig::userConfFile() {
    QStringList list = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    if (list.isEmpty()) list << QDir::homePath()+QDir::separator()+QString::fromLatin1(".config");
    return list[0]+QDir::separator()+ORG+QDir::separator()+CONF;
}

const QByteArray toByteArray(const QNetworkProxy & proxy) {
    QByteArray arr;
    QDataStream data(&arr,QIODevice::WriteOnly);
    data << (qint64)proxy.type();
    data << proxy.hostName();
    data << (qint64)proxy.port();
    data << proxy.user();
    data << proxy.password();
    return arr;
}

const QNetworkProxy toNetworkProxy(const QByteArray & arr) {
    QDataStream data(arr);
    QNetworkProxy proxy;
    QString stmp;
    qint64 itmp;
    data >> itmp;
    proxy.setType((QNetworkProxy::ProxyType)itmp);
    data >> stmp;
    proxy.setHostName(stmp);
    data >> itmp;
    proxy.setPort(itmp);
    data >> stmp;
    proxy.setUser(stmp);
    data >> stmp;
    proxy.setPassword(stmp);
    return proxy;
}

const QStringList AlpmConfig::dbExtensions() {
    return QStringList() << DBEXT << FILESEXT;
}

const QString AlpmConfig::dbExtension() {
    QSettings qalpm_ini_file(userConfFile(),QSettings::IniFormat);
    QString ext = qalpm_ini_file.value("DBExt",DBEXT).toString();
    return (dbExtensions().contains(ext)?ext:DBEXT);
}

int AlpmConfig::downloaderTimeout() {
    QSettings qalpm_ini_file(userConfFile(),QSettings::IniFormat);
    return qalpm_ini_file.value("DownloaderTimeout",30000).toInt();
}

int AlpmConfig::downloaderThreadCount() {
    QSettings qalpm_ini_file(userConfFile(),QSettings::IniFormat);
    return qalpm_ini_file.value("DownloaderThreads",get_nprocs_conf()).toInt();
}

const QNetworkProxy AlpmConfig::downloaderProxy() {
    QSettings qalpm_ini_file(userConfFile(),QSettings::IniFormat);
    return toNetworkProxy(qalpm_ini_file.value("DownloaderProxy",toByteArray(QNetworkProxy(QNetworkProxy::NoProxy))).toByteArray());
}

int AlpmConfig::useSystemIcons() {
    QSettings qalpm_ini_file(userConfFile(),QSettings::IniFormat);
    return qalpm_ini_file.value("UseSystemIcons",true).toBool();
}

bool AlpmConfig::setDBExtension(const QString & dbext) {
    QSettings qalpm_ini_file(userConfFile(),QSettings::IniFormat);
    if (!dbExtensions().contains(dbext)) return false;
    qalpm_ini_file.setValue("DBExt",dbext);
    return true;
}

void AlpmConfig::setDownloaderProxy(const QNetworkProxy & proxy) {
    QSettings qalpm_ini_file(userConfFile(),QSettings::IniFormat);
    qalpm_ini_file.setValue("DownloaderProxy",toByteArray(proxy));
}

void AlpmConfig::setDownloaderTimeout(uint value) {
    QSettings qalpm_ini_file(userConfFile(),QSettings::IniFormat);
    qalpm_ini_file.setValue("DownloaderTimeout",(int)value);
}

void AlpmConfig::setUsingSystemIcons(bool flag) {
    QSettings qalpm_ini_file(userConfFile(),QSettings::IniFormat);
    qalpm_ini_file.setValue("UseSystemIcons",flag);
}

void AlpmConfig::setDownloaderThreads(uint value) {
    QSettings qalpm_ini_file(userConfFile(),QSettings::IniFormat);
    qalpm_ini_file.setValue("DownloaderThreads",value);
}

QString AlpmConfig::confPath() const {
    return conf_filepath;
}

alpm_handle_t * AlpmConfig::translate(const QString & _dbpath) {
    if (!m_error.isEmpty()) return NULL;
    if (conf_filepath.isEmpty()) {
        m_error = QObject::tr("Pacman's conf file is unknown!");
        return NULL;
    }

    alpm_list_t * cachedirs = NULL;
    alpm_list_t * ignoregroups = NULL;
    alpm_list_t * ignorepkgs = NULL;
    alpm_list_t * noextract = NULL;
    alpm_list_t * noupgrade = NULL;
    int siglevel;
    int localfilesiglevel;
    int remotefilesiglevel;
    int i;

    m_error.clear();

    if (!_dbpath.isEmpty()) dbpath = _dbpath;
    if (!config_parse_siglevel(this->siglevel,siglevel)) return NULL;
    if (!config_parse_siglevel(this->localfilesiglevel,localfilesiglevel)) return NULL;
    if (!config_parse_siglevel(this->remotefilesiglevel,remotefilesiglevel)) return NULL;
    FREELIST(holdpkgs);
    holdpkgs = convert_list(holdpkgs2);
    cachedirs = convert_list(this->cachedirs);
    ignoregroups = convert_list(this->ignoregroups);
    ignorepkgs = convert_list(this->ignorepkgs);
    noextract = convert_list(this->noextract);
    noupgrade = convert_list(this->noupgrade);

    alpm_handle_t * m_handle = alpm_initialize(rootdir.toLocal8Bit().constData(),dbpath.toLocal8Bit().constData(),NULL);
    if(!m_handle) return NULL;

    alpm_option_set_cachedirs(m_handle,cachedirs);
    alpm_option_set_noupgrades(m_handle,noupgrade);
    alpm_option_set_noextracts(m_handle,noextract);
    alpm_option_set_ignorepkgs(m_handle,ignorepkgs);
    alpm_option_set_ignoregroups(m_handle,ignoregroups);

    FREELIST(cachedirs);
    FREELIST(noupgrade);
    FREELIST(noextract);
    FREELIST(ignorepkgs);
    FREELIST(ignoregroups);

    alpm_option_set_logfile(m_handle,logfile.toLocal8Bit().constData());
    alpm_option_set_gpgdir(m_handle,gpgdir.toLocal8Bit().constData());
    alpm_option_set_arch(m_handle,architecture.toLocal8Bit().constData());
    alpm_option_set_usesyslog(m_handle,usesyslog?1:0);
    alpm_option_set_disable_dl_timeout(m_handle,disabledownloadtimeout?1:0);

    alpm_option_set_default_siglevel(m_handle,siglevel);
    alpm_option_set_local_file_siglevel(m_handle,localfilesiglevel);
    alpm_option_set_remote_file_siglevel(m_handle,remotefilesiglevel);

    alpm_option_set_dbext(m_handle,dbExtension().toLocal8Bit().constData());

    for(i=0;i<hookdirs.count();i++) {
        alpm_option_add_hookdir(m_handle,hookdirs[i].toLocal8Bit().constData());
    }

    for (i=0;i<repos.count();i++) {
        Repo repo = repos[i];
        alpm_db_t *db = alpm_register_syncdb(m_handle,repo.name().toLocal8Bit().constData(),repo.siglevel());
        if (db) {
            alpm_db_set_servers(db,convert_list(repo.servers()));
            alpm_db_set_usage(db,repo.usage());
        }
    }

    return m_handle;
}

bool AlpmConfig::Repo::isValid() {
    return m_valid;
}

AlpmConfig::Repo::Repo(const QString & name,const QString & arch,QSettings * settings,const QStringList & def_siglevel) {
    m_valid = true;
    m_usage = 0;

    if (name.toLower() == "local") {
        m_valid = false;
        return;
    }

    QString error;
    m_name = name;
    m_arch = arch;
    QString server = settings->value(QString("%1/Server").arg(name),"").toString().replace("$repo",name).replace("$arch",arch);
    if (!server.isEmpty()) m_servers.append(server);
    server = settings->value(QString("%1/Include").arg(name),"").toString();
    if (!server.isEmpty()) setServersFromFile(server,error);
    if (!error.isEmpty()) {
        m_valid = false;
        return;
    }

    if (!config_parse_siglevel(settings->value(QString("%1/SigLevel").arg(name),def_siglevel.join(" ")).toString().trimmed().split(" ",QString::SkipEmptyParts),m_siglevel,error)) {
        m_valid = false;
        return;
    }
    QStringList values = settings->value(QString("%1/Usage").arg(name),"All").toString().trimmed().split(" ",QString::SkipEmptyParts);
    QString value;
    for (int i=0;i<values.count();i++) {
        value = values[i].toLower().trimmed();
        if (value == "all") m_usage |= ALPM_DB_USAGE_ALL;
        else if (value == "sync") m_usage |= ALPM_DB_USAGE_SYNC;
        else if (value == "search") m_usage |= ALPM_DB_USAGE_SEARCH;
        else if (value == "install") m_usage |= ALPM_DB_USAGE_INSTALL;
        else if (value == "upgrage") m_usage |= ALPM_DB_USAGE_UPGRADE;
    }

    m_valid = error.isEmpty();
}

bool AlpmConfig::Repo::config_parse_siglevel(const QStringList & values,int & level,QString & error) {
    level = ALPM_SIG_USE_DEFAULT;

    #define SET(siglevel) do { level |= (siglevel); } while(0)
    #define UNSET(siglevel) do { level &= ~(siglevel); } while(0)

    QString value;
    for (int i=0;i<values.count();i++) {
        bool pkg = true, db = true;
        value = values[i].trimmed().toLower();
        if (value.startsWith("package")) db = false;
        if (value.startsWith("database")) pkg = false;
        if (value.endsWith("never")) {
            if(pkg) { UNSET(ALPM_SIG_PACKAGE | ALPM_SIG_PACKAGE_OPTIONAL); }
            if(db) { UNSET(ALPM_SIG_DATABASE | ALPM_SIG_DATABASE_OPTIONAL); }
        }
        else if(value.endsWith("optional")) {
            if(pkg) { SET(ALPM_SIG_PACKAGE | ALPM_SIG_PACKAGE_OPTIONAL); }
            if(db) { SET(ALPM_SIG_DATABASE | ALPM_SIG_DATABASE_OPTIONAL); }
        }
        else if(value.endsWith("required")) {
            if(pkg) { SET(ALPM_SIG_PACKAGE); UNSET(ALPM_SIG_PACKAGE_OPTIONAL); }
            if(db) { SET(ALPM_SIG_DATABASE); UNSET(ALPM_SIG_DATABASE_OPTIONAL); }
        }
        else if(value.endsWith("trustedonly")) {
            if(pkg) { UNSET(ALPM_SIG_PACKAGE_MARGINAL_OK | ALPM_SIG_PACKAGE_UNKNOWN_OK); }
            if(db) { UNSET(ALPM_SIG_DATABASE_MARGINAL_OK | ALPM_SIG_DATABASE_UNKNOWN_OK); }
        }
        else if(value.endsWith("trustall")) {
            if(pkg) { SET(ALPM_SIG_PACKAGE_MARGINAL_OK | ALPM_SIG_PACKAGE_UNKNOWN_OK); }
            if(db) { SET(ALPM_SIG_DATABASE_MARGINAL_OK | ALPM_SIG_DATABASE_UNKNOWN_OK); }
        }
        else {
            error = QObject::tr("Siglevel has wrong value!");
            return false;
        }
    }

    #undef SET
    #undef UNSET

    if (level != ALPM_SIG_USE_DEFAULT) level &= ~ALPM_SIG_USE_DEFAULT;
    return true;
}

bool AlpmConfig::Repo::setServersFromFile(const QString & filepath,QString & error) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        error = QObject::tr("Can't open")+" "+filepath;
        return false;
    }

    QByteArray line;
    int index;
    while(!file.atEnd()) {
        line = file.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#") || line.startsWith(";")) continue;
        index = line.indexOf('=');
        if (index == -1) continue;
        if (line.mid(0,index).trimmed() != "Server") continue;
        m_servers.append(QString::fromLatin1(line.mid(index+1).trimmed()).replace("$repo",m_name).replace("$arch",m_arch));
    }

    return true;
}

QString AlpmConfig::lastError() const {
    return m_error;
}

