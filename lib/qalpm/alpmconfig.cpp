/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "alpmconfig.h"
#include "confreader.h"
#include <sys/utsname.h>
#include <QSettings>
#include <QFile>
#include <QTextCodec>
#include <QDir>
#include <alpm.h>
#include <QDataStream>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <pwd.h>
#include "libalpm.h"


AlpmConfig::AlpmConfig() {
    holdpkgs = nullptr;
    m_alpm_handle = nullptr;
}

AlpmConfig::~AlpmConfig() {
    FREELIST(holdpkgs);
}

alpm_list_t * AlpmConfig::convert_list(const QStringList & list) {
    alpm_list_t * ret = nullptr;

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
    architectures.clear();
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
    ConfReader ini_file(conf_filepath.isEmpty()?PACMANCONF:conf_filepath);
    if (!ini_file.lastError().isEmpty()) {
        m_error = ini_file.lastError();
        return false;
    }
    QStringList groups = ini_file.sections();
    if (!groups.contains("options")) {
        m_error = QObject::tr("Can't find [options] section!");
        return false;
    }

    rootdir = ini_file.value("options/RootDir",QString::fromLatin1("/"));
    dbpath = ini_file.value("options/DBPath",QString::fromLatin1(DBPATH));
    gpgdir = ini_file.value("options/GPGDir",QString::fromLatin1(GPGDIR));
    logfile = ini_file.value("options/LogFile",QString::fromLatin1(LOGFILE));
    architectures = split_options(ini_file.values<QString>("options/Architecture"));
    for (int i=0;i<architectures.count();i++) {
        if (architectures[i].toLower() == "auto") {
            struct utsname un;
            if(uname(&un) != 0) {
                m_error = QObject::tr("Can't determine system's architecture!");
                return false;
            }
            architectures[i] = un.machine;
        }
    }

    usesyslog = ini_file.keyExists("options/UseSyslog");
    disabledownloadtimeout = ini_file.keyExists("options/DisableDownloadTimeout");
    QStringList level = split_options(ini_file.values<QString>("options/SigLevel"));
    level.insert(0,"Optional");
    level.insert(1,"TrustedOnly");
    siglevel = Repo::ListSigLevel((uint)Repo::ListSigLevel(level));
    localfilesiglevel = Repo::ListSigLevel((uint)Repo::ListSigLevel(split_options(ini_file.values<QString>("options/LocalFileSigLevel"))));
    remotefilesiglevel = Repo::ListSigLevel((uint)Repo::ListSigLevel(split_options(ini_file.values<QString>("options/RemoteFileSigLevel"))));
    holdpkgs2 = split_options(ini_file.values<QString>("options/HoldPkg"));
    cachedirs = split_options(ini_file.values<QString>("options/CacheDir",QString::fromLocal8Bit(CACHEDIR)));
    syncfirst = split_options(ini_file.values<QString>("options/SyncFirst"));
    hookdirs = split_options(ini_file.values<QString>("options/HookDir",QString::fromLocal8Bit(HOOKDIR)));
    ignoregroups = split_options(ini_file.values<QString>("options/IgnoreGroup"));
    ignorepkgs = split_options(ini_file.values<QString>("options/IgnorePkg"));
    noextract = split_options(ini_file.values<QString>("options/NoExtract"));
    noupgrade = split_options(ini_file.values<QString>("options/NoUpgrade"));

    for (int i=0;i<groups.count();i++) {
        if (groups[i] == "options") continue;
        Repo repo(groups[i],architectures,&ini_file);
        if (!repo.isValid()) continue;
        if (index_of_repo(repo.name()) != -1) continue;
        repos.append(repo);
    }
    this->conf_filepath = ini_file.fileName();

    return true;
}

const QStringList AlpmConfig::split_options(const QStringList & list) {
    QStringList ret;
    for (const QString & str: list) {
        if (!str.trimmed().isEmpty()) ret.append(str.split(" ",Qt::SkipEmptyParts));
    }
    return ret;
}

int AlpmConfig::index_of_repo(const QString & name) {
    for (int i=0;i<repos.count();i++) {
        if (!repos[i].name().compare(name,Qt::CaseInsensitive)) return i;
    }
    return -1;
}

bool AlpmConfig::addMirrorRepo(const Repo & repo_no_servers) {
    m_error.clear();

    if (m_alpm_handle == nullptr) {
        m_error = "AlpmConfig::addMirrorRepo: "+QObject::tr("Alpm class is not initialized!");
        return false;
    }

    if (Alpm::instance()->isMethodExecuting()) {
        m_error = "AlpmConfig::addMirrorRepo: "+QObject::tr("Alpm is executing the other method!");
        return false;
    }

    Repo repo(repo_no_servers);
    ConfReader ini_file(conf_filepath);
    if (!ini_file.lastError().isEmpty()) {
        m_error = ini_file.lastError();
        return false;
    }
    if (!repo.isValid()) {
        m_error = "AlpmConfig::addNewMirrorRepo: "+QObject::tr("cannot init repo object!");
        return false;
    }

    if (index_of_repo(repo.name()) != -1) {
        m_error = QString("AlpmConfig::addNewMirrorRepo: %1 - ").arg(repo.name())+QObject::tr("repo with such a name already exists!");
        return false;
    }

    if (!repo.addServersFromFile(MIRRORLIST,m_error)) return false;

    if (!ini_file.setValues(QString("%1/Include").arg(repo.name()),QStringList() << MIRRORLIST) ||
       (!repo.siglevel().isEmpty() && !ini_file.addValue(QString("%1/SigLevel").arg(repo.name()),repo.siglevel().toString())) ||
        !ini_file.addValue(QString("%1/Usage").arg(repo.name()),repo.usage().toString()) ||
        !ini_file.sync()) {
        m_error = ini_file.lastError();
        return false;
    }

    Alpm::instance()->add_sync_db(repo);
    repos.append(repo);

    return true;
}

bool AlpmConfig::addNewRepo(const Repo & repo) {
    m_error.clear();

    if (m_alpm_handle == nullptr) {
        m_error = "AlpmConfig::addNewRepo: "+QObject::tr("Alpm class is not initialized!");
        return false;
    }

    if (Alpm::instance()->isMethodExecuting()) {
        m_error = "AlpmConfig::addNewRepo: "+QObject::tr("Alpm is executing the other method!");
        return false;
    }

    ConfReader ini_file(conf_filepath);
    if (!ini_file.lastError().isEmpty()) {
        m_error = ini_file.lastError();
        return false;
    }
    if (!repo.isValid()) {
        m_error = "AlpmConfig::addNewRepo: "+QObject::tr("cannot init repo object!");
        return false;
    }

    if (index_of_repo(repo.name()) != -1) {
        m_error = QString("AlpmConfig::addNewRepo: %1 - ").arg(repo.name())+QObject::tr("repo with such a name already exists!");
        return false;
    }

    if (!ini_file.setValues(QString("%1/Server").arg(repo.name()),repo.servers()) ||
       (!repo.siglevel().isEmpty() && !ini_file.addValue(QString("%1/SigLevel").arg(repo.name()),repo.siglevel().toString())) ||
        !ini_file.addValue(QString("%1/Usage").arg(repo.name()),repo.usage().toString()) ||
        !ini_file.sync()) {
        m_error = ini_file.lastError();
        return false;
    }

    if (!Alpm::instance()->add_sync_db(repo)) {
        m_error = Alpm::instance()->lastError();
        return false;
    }
    repos.append(repo);

    return true;
}

bool AlpmConfig::deleteRepo(const QString & name) {
    m_error.clear();

    if (m_alpm_handle == nullptr) {
        m_error = "AlpmConfig::deleteRepo: "+QObject::tr("Alpm class is not initialized!");
        return false;
    }

    if (Alpm::instance()->isMethodExecuting()) {
        m_error = "AlpmConfig::deleteRepo: "+QObject::tr("Alpm is executing the other method!");
        return false;
    }

    if (repos.count() <= 1) {
        m_error = QString("AlpmConfig::deleteRepo: ")+QObject::tr("must stay at least one repo!");
        return false;
    }

    int index;
    if ((index = index_of_repo(name)) == -1) {
        m_error = QString("AlpmConfig::deleteRepo: %1 - ").arg(name.toLower())+QObject::tr("repo with such a name does not exist!");
        return false;
    }

    ConfReader ini_file(conf_filepath);
    if (!ini_file.lastError().isEmpty()) {
        m_error = ini_file.lastError();
        return false;
    }
    ini_file.remove(repos[index].m_orig_name+"/");
    if (!ini_file.sync()) {
        m_error = ini_file.lastError();
        return false;
    }

    Alpm::instance()->remove_sync_db(name);
    repos.removeAt(index);

    return true;
}

const QString AlpmConfig::user_name() {
    uid_t uid = geteuid();
    struct passwd * pw = (uid == (uid_t)-1 && errno ? nullptr : getpwuid(uid));
    if (pw == nullptr) return QString();

    return QString::fromLocal8Bit(pw->pw_name);
}

const QString AlpmConfig::user_dir() {
    uid_t uid = geteuid();
    struct passwd * pw = (uid == (uid_t)-1 && errno ? nullptr : getpwuid(uid));
    if (pw == nullptr) return QString();

    return QString::fromLocal8Bit(pw->pw_dir);
}

const QString AlpmConfig::user_conf_file() {
    QString configdir;
    QString username = user_name();
    QString userdir = user_dir();
    if (username.isEmpty() || userdir.isEmpty()) configdir = QDir::separator() + QLatin1String(".config");
    else configdir = userdir + QDir::separator() +".config";

    return configdir+QDir::separator()+ORG+QDir::separator()+CONF;
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
    QSettings qalpm_ini_file(user_conf_file(),QSettings::IniFormat);
    QString ext = qalpm_ini_file.value("DBExt",DBEXT).toString();
    return (dbExtensions().contains(ext)?ext:DBEXT);
}

uint AlpmConfig::downloaderTimeout() {
    QSettings qalpm_ini_file(user_conf_file(),QSettings::IniFormat);
    return qalpm_ini_file.value("DownloaderTimeout",30000).toUInt();
}

uint AlpmConfig::downloaderThreadCount() {
    QSettings qalpm_ini_file(user_conf_file(),QSettings::IniFormat);
    return qalpm_ini_file.value("DownloaderThreads",get_nprocs_conf()).toUInt();
}

const QNetworkProxy AlpmConfig::downloaderProxy() {
    QSettings qalpm_ini_file(user_conf_file(),QSettings::IniFormat);
    return toNetworkProxy(qalpm_ini_file.value("DownloaderProxy",toByteArray(QNetworkProxy(QNetworkProxy::NoProxy))).toByteArray());
}

bool AlpmConfig::useSystemIcons() {
    QSettings qalpm_ini_file(user_conf_file(),QSettings::IniFormat);
    return qalpm_ini_file.value("UseSystemIcons",true).toBool();
}

bool AlpmConfig::setDBExtension(const QString & dbext) {
    QSettings qalpm_ini_file(user_conf_file(),QSettings::IniFormat);
    if (!dbExtensions().contains(dbext)) return false;
    qalpm_ini_file.setValue("DBExt",dbext);
    return true;
}

void AlpmConfig::setDownloaderProxy(const QNetworkProxy & proxy) {
    QSettings qalpm_ini_file(user_conf_file(),QSettings::IniFormat);
    qalpm_ini_file.setValue("DownloaderProxy",toByteArray(proxy));
}

void AlpmConfig::setDownloaderTimeout(uint value) {
    QSettings qalpm_ini_file(user_conf_file(),QSettings::IniFormat);
    qalpm_ini_file.setValue("DownloaderTimeout",value);
}

void AlpmConfig::setUsingSystemIcons(bool flag) {
    QSettings qalpm_ini_file(user_conf_file(),QSettings::IniFormat);
    qalpm_ini_file.setValue("UseSystemIcons",flag);
}

void AlpmConfig::setDownloaderThreads(uint value) {
    QSettings qalpm_ini_file(user_conf_file(),QSettings::IniFormat);
    qalpm_ini_file.setValue("DownloaderThreads",value);
}

QString AlpmConfig::confPath() const {
    return conf_filepath;
}

alpm_handle_t * AlpmConfig::translate(const QString & _dbpath) {
    if (!m_error.isEmpty()) return nullptr;
    if (conf_filepath.isEmpty()) {
        m_error = QObject::tr("Pacman's conf file is unknown!");
        return nullptr;
    }

    alpm_list_t * cachedirs = nullptr;
    alpm_list_t * ignoregroups = nullptr;
    alpm_list_t * ignorepkgs = nullptr;
    alpm_list_t * noextract = nullptr;
    alpm_list_t * noupgrade = nullptr;
    int i;

    m_error.clear();

    if (!_dbpath.isEmpty()) dbpath = _dbpath;
    FREELIST(holdpkgs);
    holdpkgs = convert_list(holdpkgs2);
    cachedirs = convert_list(this->cachedirs);
    ignoregroups = convert_list(this->ignoregroups);
    ignorepkgs = convert_list(this->ignorepkgs);
    noextract = convert_list(this->noextract);
    noupgrade = convert_list(this->noupgrade);

    alpm_handle_t * m_handle = alpm_initialize(rootdir.toLocal8Bit().constData(),dbpath.toLocal8Bit().constData(),nullptr);
    if(!m_handle) return nullptr;

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
    for (QString & architecture: architectures) alpm_option_add_architecture(m_handle,architecture.toLocal8Bit().constData());
    alpm_option_set_usesyslog(m_handle,usesyslog?1:0);
    alpm_option_set_disable_dl_timeout(m_handle,disabledownloadtimeout?1:0);

    alpm_option_set_default_siglevel(m_handle,siglevel);
    alpm_option_set_local_file_siglevel(m_handle,localfilesiglevel);
    alpm_option_set_remote_file_siglevel(m_handle,remotefilesiglevel);

    alpm_option_set_dbext(m_handle,dbExtension().toLocal8Bit().constData());

    if (dbExtension() == DBEXT) {
        QDir dir(dbpath+QDir::separator()+"sync");
        dir.setNameFilters(QStringList() << QString("*%1").arg(FILESEXT));
        for (QString & fileName: dir.entryList()) {
            QFile(dbpath+QDir::separator()+"sync"+QDir::separator()+fileName).remove();
        }
    }

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

    m_alpm_handle = m_handle;
    return m_handle;
}

bool AlpmConfig::Repo::isValid() const {
    return m_valid;
}

QDataStream & operator<<(QDataStream &argument,const AlpmConfig::Repo::SigLevel & level) {
    argument << (uint)level.object();
    argument << (uint)level.check();
    argument << (uint)level.allowed();

    return argument;
}

const QDataStream & operator>>(const QDataStream &argument,AlpmConfig::Repo::SigLevel & level) {
    uint val;
    (QDataStream &)argument >> val;
    level.m_object = (AlpmConfig::Repo::SigObject)val;
    (QDataStream &)argument >> val;
    level.m_check = (AlpmConfig::Repo::SigCheck)val;
    (QDataStream &)argument >> val;
    level.m_allowed = (AlpmConfig::Repo::SigAllowed)val;

    return argument;
}

QDataStream & operator<<(QDataStream &argument,const AlpmConfig::Repo::Usage & usage) {
    argument << (int)usage.isSync();
    argument << (int)usage.isSearch();
    argument << (int)usage.isInstall();
    argument << (int)usage.isUpgrade();

    return argument;
}

const QDataStream & operator>>(const QDataStream &argument,AlpmConfig::Repo::Usage & usage) {
    int val;
    (QDataStream &)argument >> val;
    usage.m_sync = (bool)val;
    (QDataStream &)argument >> val;
    usage.m_search = (bool)val;
    (QDataStream &)argument >> val;
    usage.m_install = (bool)val;
    (QDataStream &)argument >> val;
    usage.m_upgrade = (bool)val;

    return argument;
}

QDataStream & operator<<(QDataStream &argument,const AlpmConfig::Repo & repo) {
    argument << repo.usage();
    argument << repo.name();
    argument << repo.arches();
    argument << repo.servers();
    argument << repo.siglevel();
    argument << (bool)repo.m_valid;

    return argument;
}

const QDataStream & operator>>(const QDataStream &argument,AlpmConfig::Repo & repo) {
    (QDataStream &)argument >> repo.m_usage;
    (QDataStream &)argument >> repo.m_name;
    repo.m_orig_name = repo.m_name;
    (QDataStream &)argument >> repo.m_arches;
    (QDataStream &)argument >> repo.m_servers;
    (QDataStream &)argument >> repo.m_siglevel;
    (QDataStream &)argument >> repo.m_valid;

    return argument;
}

AlpmConfig::Repo::Repo(const QString & name,const QStringList & urls, const QStringList & arches,const ListSigLevel & siglevels,const Usage & usages) {
    m_valid = true;
    m_orig_name = name;
    m_name = name.toLower();
    if (m_name == "local") {
        m_valid = false;
        return;
    }

    m_arches = arches;
    m_servers = urls;
    for (QString & server: m_servers) {
        for (const QString & arch: arches) server = server.replace("$repo",m_name).replace("$arch",arch);
    }
    m_servers.removeDuplicates();

    m_usage = usages;
    m_siglevel = ListSigLevel((uint)siglevels);
}

AlpmConfig::Repo::Repo() {
    m_valid = false;
}

AlpmConfig::Repo::Repo(const AlpmConfig::Repo & repo) {
    *this = repo;
}

AlpmConfig::Repo::Repo(const QString & name, const QStringList & arches, ConfReader * settings) {
    m_valid = true;

    m_orig_name = name;
    m_name = name.toLower();
    if (m_name == "local") {
        m_valid = false;
        return;
    }

    QString error = "";
    m_arches = arches;
    m_servers = AlpmConfig::split_options(settings->values<QString>(QString("%1/Server").arg(m_name)));
    for (QString & server: m_servers) {
        for (const QString & arch: arches) server = server.replace("$repo",m_name).replace("$arch",arch);
    }
    m_servers.removeDuplicates();
    QString include = settings->value(QString("%1/Include").arg(m_name),QString(""));
    if (!include.isEmpty()) addServersFromFile(include,error);
    if (!error.isEmpty()) {
        m_valid = false;
        return;
    }

    m_siglevel = ListSigLevel((uint)ListSigLevel(AlpmConfig::split_options(settings->values<QString>(QString("%1/SigLevel").arg(m_name)))));
    m_usage = Usage(AlpmConfig::split_options(settings->values<QString>(QString("%1/Usage").arg(m_name),QString::fromLatin1("All"))));
    m_valid = error.isEmpty();
}

AlpmConfig::Repo & AlpmConfig::Repo::operator=(const AlpmConfig::Repo &repo) {
    m_orig_name = repo.m_orig_name;
    m_valid = repo.m_valid;
    m_usage = repo.m_usage;
    m_name = repo.m_name;
    m_arches = repo.m_arches;
    m_servers = repo.m_servers;
    m_siglevel = repo.m_siglevel;
    return *this;
}

AlpmConfig::Repo::Usage::Usage() {
    m_sync = true;
    m_search = true;
    m_install = true;
    m_upgrade = true;
}

AlpmConfig::Repo::Usage::Usage(const QStringList & values) {
    if (values.isEmpty() || values.contains("all",Qt::CaseInsensitive)) {
        m_sync = true;
        m_search = true;
        m_install = true;
        m_upgrade = true;
        return;
    }
    m_sync = false;
    m_search = false;
    m_install = false;
    m_upgrade = false;
    if (values.contains("sync",Qt::CaseInsensitive)) m_sync = true;
    if (values.contains("search",Qt::CaseInsensitive)) m_search = true;
    if (values.contains("install",Qt::CaseInsensitive)) m_install = true;
    if (values.contains("upgrade",Qt::CaseInsensitive)) m_upgrade = true;
}

AlpmConfig::Repo::Usage::Usage(uint val) {
    if (AlpmConfig::test_flag(val,ALPM_DB_USAGE_ALL)) {
        m_sync = true;
        m_search = true;
        m_install = true;
        m_upgrade = true;
        return;
    }
    m_sync = false;
    m_search = false;
    m_install = false;
    m_upgrade = false;
    if (AlpmConfig::test_flag(val,ALPM_DB_USAGE_SYNC)) m_sync = true;
    if (AlpmConfig::test_flag(val,ALPM_DB_USAGE_SEARCH)) m_search = true;
    if (AlpmConfig::test_flag(val,ALPM_DB_USAGE_INSTALL)) m_install = true;
    if (AlpmConfig::test_flag(val,ALPM_DB_USAGE_UPGRADE)) m_upgrade = true;
}

bool AlpmConfig::Repo::Usage::isAll() const {
    return ((m_sync && m_search && m_install && m_upgrade) || (!m_sync && !m_search && !m_install && !m_upgrade));
}

AlpmConfig::Repo::Usage::operator uint() const {
    if (isAll()) return ALPM_DB_USAGE_ALL;
    uint ret = 0;
    if (m_sync) ret |= ALPM_DB_USAGE_SYNC;
    if (m_search) ret |= ALPM_DB_USAGE_SEARCH;
    if (m_install) ret |= ALPM_DB_USAGE_INSTALL;
    if (m_upgrade) ret |= ALPM_DB_USAGE_UPGRADE;
    return ret;
}

QString AlpmConfig::Repo::Usage::toString() const {
    if (isAll()) return "All";
    QString ret;
    if (m_sync) ret += " Sync";
    if (m_search) ret += " Search";
    if (m_install) ret += " Install";
    if (m_upgrade) ret += " Upgrade";
    return ret.trimmed();
}

AlpmConfig::Repo::SigLevel::SigLevel() {
    m_object = Both;
    m_check = Default;
    m_allowed = Nothing;
}

AlpmConfig::Repo::SigLevel::SigLevel(SigObject object,SigCheck check,SigAllowed allowed) {
    m_object = object;
    m_check = check;
    m_allowed = allowed;
}

QString AlpmConfig::Repo::SigLevel::toString() const {
    QString ret;
    QString pkg;
    switch (m_object) {
    case Package:
        pkg="Package";
        break;
    case Database:
        pkg="Database";
        break;
    case Both:
        break;
    }
    switch (m_check) {
    case Optional:
        ret+=pkg+"Optional";
        break;
    case Required:
        ret+=pkg+"Required";
        break;
    case Never:
        ret+=pkg+"Never";
        break;
    case Default:
        break;
    }
    switch (m_allowed) {
    case TrustedOnly:
        ret+=(ret.isEmpty()?"":" ") + pkg+"TrustedOnly";
        break;
    case TrustAll:
        ret+=(ret.isEmpty()?"":" ") + pkg+"TrustAll";
        break;
    case Nothing:
        break;
    }
    return ret;
}

bool AlpmConfig::Repo::SigLevel::operator==(const SigLevel & other) const {
    return (m_object == other.m_object && m_check == other.m_check && m_allowed == other.m_allowed);
}

bool AlpmConfig::test_flag(uint level,uint flag) {
    return ((level & flag) == flag);
}

AlpmConfig::Repo::ListSigLevel::ListSigLevel(uint level) : QList<SigLevel>() {
    if (AlpmConfig::test_flag(level,ALPM_SIG_USE_DEFAULT)) return;

    bool is_set = false;
    SigLevel siglevel;
    if (AlpmConfig::test_flag(level,ALPM_SIG_PACKAGE | ALPM_SIG_PACKAGE_OPTIONAL | ALPM_SIG_DATABASE | ALPM_SIG_DATABASE_OPTIONAL)) {
        siglevel.m_object = Both;
        siglevel.m_check = Optional;
        is_set = true;
    }
    else if (AlpmConfig::test_flag(level,ALPM_SIG_PACKAGE | ALPM_SIG_DATABASE)) {
        siglevel.m_object = Both;
        siglevel.m_check = Required;
        is_set = true;
    }

    if (!is_set && AlpmConfig::test_flag(level,ALPM_SIG_PACKAGE | ALPM_SIG_PACKAGE_OPTIONAL)) {
        siglevel.m_object = Package;
        siglevel.m_check = Optional;
        is_set = true;
    }
    else if (!is_set && AlpmConfig::test_flag(level,ALPM_SIG_PACKAGE)) {
        siglevel.m_object = Package;
        siglevel.m_check = Required;
        is_set = true;
    }

    if (!is_set && AlpmConfig::test_flag(level,ALPM_SIG_DATABASE | ALPM_SIG_DATABASE_OPTIONAL)) {
        siglevel.m_object = Database;
        siglevel.m_check = Optional;
        is_set = true;
    }
    else if (!is_set && AlpmConfig::test_flag(level,ALPM_SIG_DATABASE)) {
        siglevel.m_object = Database;
        siglevel.m_check = Required;
        is_set = true;
    }

    if (is_set) append(siglevel);
    SigLevel siglevel2;
    if (!AlpmConfig::test_flag(level,ALPM_SIG_PACKAGE) || !AlpmConfig::test_flag(level,ALPM_SIG_DATABASE)) {
        siglevel.m_check = Never;
        siglevel2.m_check = Required;
        siglevel2.m_object = Database;
        SigLevel siglevel3;
        siglevel3.m_check = Optional;
        siglevel3.m_object = Database;
        if (contains(siglevel2) || contains(siglevel3)) siglevel.m_object = Package;
        else {
             siglevel2.m_object = Package;
             siglevel3.m_object = Package;
             if (contains(siglevel2) || contains(siglevel3)) siglevel.m_object = Database;
             else siglevel.m_object = Both;
        }
        append(siglevel);
    }

    is_set = false;
    siglevel.m_check = Default;
    if (AlpmConfig::test_flag(level,ALPM_SIG_PACKAGE_MARGINAL_OK | ALPM_SIG_PACKAGE_UNKNOWN_OK | ALPM_SIG_DATABASE_MARGINAL_OK | ALPM_SIG_DATABASE_UNKNOWN_OK)) {
        siglevel.m_allowed = TrustAll;
        is_set = true;
    }
    else if (AlpmConfig::test_flag(level,ALPM_SIG_PACKAGE_MARGINAL_OK | ALPM_SIG_PACKAGE_UNKNOWN_OK)) {
        siglevel.m_object = Package;
        siglevel.m_allowed = TrustAll;
        is_set = true;
    }
    else if (AlpmConfig::test_flag(level,ALPM_SIG_DATABASE_MARGINAL_OK | ALPM_SIG_DATABASE_UNKNOWN_OK)) {
        siglevel.m_object = Database;
        siglevel.m_allowed = TrustAll;
        is_set = true;
    }

    if (is_set) append(siglevel);
    if (AlpmConfig::test_flag(level,ALPM_SIG_PACKAGE) || AlpmConfig::test_flag(level,ALPM_SIG_DATABASE)) {
        siglevel.m_allowed = TrustedOnly;
        siglevel2.m_check = Default;
        siglevel2.m_allowed = TrustAll;
        siglevel2.m_object = Database;
        if (contains(siglevel2)) siglevel.m_object = Package;
        else {
             siglevel2.m_object = Package;
             if (contains(siglevel2)) siglevel.m_object = Database;
             else siglevel.m_object = Both;
        }
        append(siglevel);
    }
}

void AlpmConfig::Repo::ListSigLevel::siglevel_to_int(uint & ret,bool is_package,SigCheck check,SigAllowed allowed) const {
    switch (check) {
    case AlpmConfig::Repo::Optional:
        ret |= is_package?(ALPM_SIG_PACKAGE | ALPM_SIG_PACKAGE_OPTIONAL):(ALPM_SIG_DATABASE | ALPM_SIG_DATABASE_OPTIONAL);
        break;
    case AlpmConfig::Repo::Required:
        ret |= is_package?ALPM_SIG_PACKAGE:ALPM_SIG_DATABASE;
        ret &= ~(is_package?ALPM_SIG_PACKAGE_OPTIONAL:ALPM_SIG_DATABASE_OPTIONAL);
        break;
    case AlpmConfig::Repo::Never:
        ret &= ~(is_package?(ALPM_SIG_PACKAGE | ALPM_SIG_PACKAGE_OPTIONAL):(ALPM_SIG_DATABASE | ALPM_SIG_DATABASE_OPTIONAL));
        break;
    case AlpmConfig::Repo::Default:
        break;
    }

    switch (allowed) {
    case AlpmConfig::Repo::TrustedOnly:
        ret &= ~(is_package?(ALPM_SIG_PACKAGE_MARGINAL_OK | ALPM_SIG_PACKAGE_UNKNOWN_OK):(ALPM_SIG_DATABASE_MARGINAL_OK | ALPM_SIG_DATABASE_UNKNOWN_OK));
        break;
    case AlpmConfig::Repo::TrustAll:
        ret |= (is_package && AlpmConfig::test_flag(ret,ALPM_SIG_PACKAGE))?(ALPM_SIG_PACKAGE_MARGINAL_OK | ALPM_SIG_PACKAGE_UNKNOWN_OK):((!is_package && AlpmConfig::test_flag(ret,ALPM_SIG_DATABASE))?(ALPM_SIG_DATABASE_MARGINAL_OK | ALPM_SIG_DATABASE_UNKNOWN_OK):0);
        break;
    case AlpmConfig::Repo::Nothing:
        break;
    }
}

QString AlpmConfig::Repo::ListSigLevel::toString() const {
    QString ret;
    for (const SigLevel & level: *this) {
        ret += " " + level.toString();
    }

    return ret.trimmed();
}

QStringList AlpmConfig::Repo::ListSigLevel::toStringList() const {
    QStringList ret;
    for (const SigLevel & level: *this) {
        ret.append(level.toString());
    }

    return ret;
}

AlpmConfig::Repo::ListSigLevel::operator uint() const {
    if (isEmpty()) return ALPM_SIG_USE_DEFAULT;
    uint ret = 0;
    for (const AlpmConfig::Repo::SigLevel & siglevel: *this) {
        switch (siglevel.m_object) {
        case AlpmConfig::Repo::Package:
            siglevel_to_int(ret,true,siglevel.m_check,siglevel.m_allowed);
            break;
        case AlpmConfig::Repo::Database:
            siglevel_to_int(ret,false,siglevel.m_check,siglevel.m_allowed);
            break;
        case AlpmConfig::Repo::Both:
            siglevel_to_int(ret,true,siglevel.m_check,siglevel.m_allowed);
            siglevel_to_int(ret,false,siglevel.m_check,siglevel.m_allowed);
            break;
        }
    }

    return ret;
}

AlpmConfig::Repo::ListSigLevel::ListSigLevel(const QStringList & values) {
    QString value;
    SigLevel siglevel;
    for (int i=0;i<values.count();i++) {
        siglevel.m_check = Default;
        siglevel.m_allowed = Nothing;
        value = values[i].trimmed().toLower();
        if (value.startsWith("package")) siglevel.m_object = Package;
        else if (value.startsWith("database")) siglevel.m_object = Database;
        else siglevel.m_object = Both;
        if (value.endsWith("never")) siglevel.m_check = Never;
        else if(value.endsWith("optional")) siglevel.m_check = Optional;
        else if(value.endsWith("required")) siglevel.m_check = Required;
        else if(value.endsWith("trustedonly")) siglevel.m_allowed = TrustedOnly;
        else if(value.endsWith("trustall")) siglevel.m_allowed = TrustAll;
        append(siglevel);
    }
}

bool AlpmConfig::Repo::addServersFromFile(const QString & filepath,QString & error) {
    ConfReader mirrors(filepath);
    if (!mirrors.lastError().isEmpty()) {
        error = mirrors.lastError();
        return false;
    }

    m_servers = mirrors.values<QString>("/Server");
    for (QString & server: m_servers) {
        for (QString & arch: m_arches)  server = server.replace("$repo",m_name).replace("$arch",arch);
    }
    m_servers.removeDuplicates();

    return true;
}

QString AlpmConfig::lastError() const {
    return m_error;
}

