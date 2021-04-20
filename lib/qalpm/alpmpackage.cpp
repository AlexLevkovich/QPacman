/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "alpmpackage.h"
#include "libalpm.h"
#include "alpmdb.h"
#include <QByteArray>
#include <QRectF>
#include <QPointF>
#include <QFileInfo>
#include <AppStreamQt/pool.h>
#include <AppStreamQt/image.h>
#include <AppStreamQt/icon.h>
#include <searchpackage.h>
#include <QDebug>
#include <limits>
#include "archivefilesiterator.h"

AppStream::Pool * AlpmPackage::m_pool = NULL;
QMultiHash<QString, AppStream::Component> AlpmPackage::m_appInfo;
QMap<AlpmPackage::alpm_pkg_t_p,AlpmPackage::UserChangeStatus> AlpmPackage::m_change_statuses;
const int metareg = AlpmPackage::registerMetaType();

static const qint64 QINT64_MAX = std::numeric_limits<qint64>::max();
static const qint64 QINT64_MIN = std::numeric_limits<qint64>::min();

class Line {
public:
    Line(const AlpmPackage::Dependence & dep) {
        p1 = QINT64_MIN;
        p2 = QINT64_MAX;
        switch (dep.operation()) {
        case AlpmPackage::LESS:
        case AlpmPackage::LESS_OR_EQUAL:
            p2 = (qint64)qHash(dep.version());
            break;
        case AlpmPackage::MORE:
        case AlpmPackage::MORE_OR_EQUAL:
            p1 = (qint64)qHash(dep.version());
            break;
        case AlpmPackage::EQUAL:
            p1 = p2 = (qint64)qHash(dep.version());
            break;
        default:
            break;
        }
    }

    qint64 value() {
        return (p1 == QINT64_MIN)?p2:p1;
    }

    bool intersects(const Line &l) {
        return (p1 < l.p2 && l.p1 < p2);
    }
private:
    qint64 p1;
    qint64 p2;
};

static int compare_deps(const AlpmPackage::Dependence & dep1,const AlpmPackage::Dependence & dep2) {
    int ret;
    if ((ret = dep1.name().compare(dep2.name())) != 0) return ret;


    Line line1(dep1);
    Line line2(dep2);

    if (line1.intersects(line2)) return 0;

    return (int)(line1.value() - line2.value());
}

bool operator<(const AlpmPackage::Dependence & dep1, const AlpmPackage::Dependence & dep2) {
    return (compare_deps(dep1,dep2) < 0);
}

AlpmPackage::Dependence::Dependence() {
    init("",UNKNOWN);
}

AlpmPackage::Dependence::Dependence(const AlpmPackage::Dependence & dep) {
    init(dep.name(),dep.operation(),dep.version(),dep.description());
}

AlpmPackage::Dependence::Dependence(const QString & name,CompareOper operation,const QString & version,const QString & description) {
    init(name,operation,version,description);
}

AlpmPackage::Dependence::Dependence(const AlpmPackage & pkg,CompareOper operation) {
    init(pkg,operation);
}

void AlpmPackage::Dependence::init(const QString & name,CompareOper operation,const QString & version,const QString & description) {
    m_name = name;
    m_version = version;
    m_description = description;
    m_operation = operation;
    if (m_operation == UNKNOWN) m_version.clear();
}

void AlpmPackage::Dependence::init(const AlpmPackage & pkg,CompareOper operation) {
    init(pkg.name(),operation,pkg.version(),pkg.description());
}

AlpmPackage::Dependence & AlpmPackage::Dependence::operator=(const AlpmPackage::Dependence &dep) {
    m_name = dep.name();
    m_version = dep.version();
    m_description = dep.description();
    m_operation = dep.operation();
    return *this;
}

QString AlpmPackage::Dependence::name() const {
    return m_name;
}

QString AlpmPackage::Dependence::version() const {
    return m_version;
}

QString AlpmPackage::Dependence::description() const {
    return m_description;
}

class depend_t : public alpm_depend_t {
public:
    depend_t(const QString & name,const QString & version,const QString & desc,AlpmPackage::CompareOper mod) {
        this->name = strdup(name.toLatin1().constData());
        this->version = strdup(version.toLatin1().constData());
        this->desc = strdup(desc.toLocal8Bit().constData());
        this->mod = compareoper_to_mod(mod);
        this->name_hash = hash_from_name();
    }

    depend_t(const depend_t & dep) {
        this->name = strdup(dep.name);
        this->version = strdup(dep.version);
        this->desc = strdup(dep.desc);
        this->mod = dep.mod;
        this->name_hash = dep.name_hash;
    }

    depend_t(const AlpmPackage::Dependence & dep) {
        this->name = strdup(dep.name().toLatin1().constData());
        this->version = strdup(dep.version().toLatin1().constData());
        this->desc = strdup(dep.description().toLocal8Bit().constData());
        this->mod = compareoper_to_mod(dep.operation());
        this->name_hash = hash_from_name();
    }

private:
    static alpm_depmod_t compareoper_to_mod(AlpmPackage::CompareOper mod) {
        switch (mod) {
        case AlpmPackage::UNKNOWN:
            return ALPM_DEP_MOD_ANY;
        case AlpmPackage::EQUAL:
            return ALPM_DEP_MOD_EQ;
        case AlpmPackage::MORE_OR_EQUAL:
            return ALPM_DEP_MOD_GE;
        case AlpmPackage::LESS_OR_EQUAL:
            return ALPM_DEP_MOD_LE;
        case AlpmPackage::MORE:
            return ALPM_DEP_MOD_GT;
        case AlpmPackage::LESS:
            return ALPM_DEP_MOD_LT;
        default:
            break;
        }
        return ALPM_DEP_MOD_ANY;
    }

    unsigned long hash_from_name() const {
        unsigned long hash = 0;
        int c;

        const char * str = name;

        if(!str) {
            return hash;
        }
        while((c = *str++)) {
            hash = c + hash * 65599;
        }

        return hash;
    }
};


AlpmPackage::CompareOper AlpmPackage::Dependence::mod_to_compareoper(alpm_depmod_t mod) {
    switch (mod) {
    case ALPM_DEP_MOD_ANY:
        return AlpmPackage::UNKNOWN;
    case ALPM_DEP_MOD_EQ:
        return AlpmPackage::EQUAL;
    case ALPM_DEP_MOD_GE:
        return AlpmPackage::MORE_OR_EQUAL;
    case ALPM_DEP_MOD_LE:
        return AlpmPackage::LESS_OR_EQUAL;
    case ALPM_DEP_MOD_GT:
        return AlpmPackage::MORE;
    case ALPM_DEP_MOD_LT:
        return AlpmPackage::LESS;
    default:
        break;
    }
    return AlpmPackage::UNKNOWN;
}

alpm_depend_t AlpmPackage::Dependence::to_alpm_depend() const {
    return depend_t(*this);
}

AlpmPackage::CompareOper AlpmPackage::Dependence::operation() const {
    return m_operation;
}

QString AlpmPackage::Dependence::toString() const {
    QString oper;
    switch (m_operation) {
    case UNKNOWN:
        oper = "-";
        break;
    case MORE:
        oper = ">";
        break;
    case MORE_OR_EQUAL:
        oper = ">=";
        break;
    case LESS:
        oper = "<";
        break;
    case LESS_OR_EQUAL:
        oper = "<=";
        break;
    case EQUAL:
        oper = "=";
        break;
    }
    return m_version.isEmpty()?m_name:m_name+oper+m_version;
}

bool AlpmPackage::Dependence::isAppropriate(const AlpmPackage & pkg) const {
    if (!pkg.isValid()) return false;

    return isAppropriate(pkg.name(),pkg.version());
}

const AlpmPackage::Dependence AlpmPackage::Dependence::fromString(const QString & str) {
    QString name;
    QString ver;
    AlpmPackage::CompareOper oper = AlpmPackage::parseNameVersion(str,name,ver);
    return AlpmPackage::Dependence(name,oper,ver);
}

bool AlpmPackage::Dependence::isAppropriate(const Dependence & dep) const {
    return !compare_deps(*this,dep);
}

bool AlpmPackage::Dependence::isAppropriate(const QString & name,const QString & version) const {
    if (m_name != name) return false;
    if (m_operation == UNKNOWN) return true;
    int ret = AlpmPackage::pkg_vercmp(version,m_version);
    switch (m_operation) {
    case MORE:
        return (ret > 0);
    case LESS:
        return (ret < 0);
    case EQUAL:
        return (ret == 0);
    case MORE_OR_EQUAL:
        return (ret >= 0);
    case LESS_OR_EQUAL:
        return (ret <= 0);
    default:
        break;
    }
    return true;
}

bool AlpmPackage::Dependence::operator==(const AlpmPackage::Dependence & dep) {
    return !compare_deps(*this,dep);
}

bool AlpmPackage::Dependence::operator<(const Dependence & dep) {
    return (compare_deps(*this,dep) < 0);
}

QList<AlpmPackage::Dependence> AlpmPackage::Dependence::findDepends(uint provider_id) const {
    QList<AlpmPackage::Dependence> ret;
    Alpm * alpm = Alpm::instance();
    if (alpm == NULL) return ret;

    QList<AlpmPackage> pkgs = version().isEmpty()?alpm->findByPackageName(name()):alpm->findByPackageNameVersion(name(),version());
    if (pkgs.count() <= 0) {
        pkgs = alpm->findByPackageNameProvides(*this);
        if (pkgs.count() <= 0) return ret;
    }

    for (int i=0;i<pkgs.count();i++) {
        if (pkgs[i].isInstalled()) return pkgs[i].depends();
    }
    return pkgs[((int)provider_id >= pkgs.count())?(pkgs.count()-1):(int)provider_id].depends();
}

bool AlpmPackage::Dependence::isInstalled() const {
    Alpm * alpm = Alpm::instance();
    if (alpm == NULL) return false;
    AlpmDB db = alpm->localDB();
    if (!db.findByPackageName(m_name).isValid()) {
        return db.findByPackageNameProvides(*this).count() > 0;
    }
    return true;
}

AlpmPackage::AlpmPackage() {
    m_handle = NULL;
    m_delete = true;
}

AlpmPackage::AlpmPackage(alpm_pkg_t *pkg) {
    m_handle = pkg;
    m_delete = true;
}

const AlpmPackage AlpmPackage::newPackage(const QString & name,const QString & version) {
    AlpmPackage pkg;
    SearchPackage key(name,version);
    key.setAutoFree(false);
    pkg.m_handle = key.packageHandle();
    pkg.m_delete = true;
    return pkg;
}

AlpmPackage::AlpmPackage(const QString & name,const QString & version,const QString & dbname) {
    m_handle = NULL;
    m_delete = true;
    if (Alpm::p_alpm == NULL || Alpm::p_alpm->m_alpm_handle == NULL) return;
    AlpmDB db(dbname);
    if (!db.isValid()) return;
    m_handle = alpm_db_get_pkg(db.m_db_handle,name.toLocal8Bit().constData());
    if (m_handle == NULL) return;
    if (QString::fromLocal8Bit(alpm_pkg_get_version(m_handle)) != version) {
        m_handle = NULL;
        return;
    }
}

AlpmPackage::AlpmPackage(const QString & filename,bool do_delete) {
    m_delete = do_delete;
    m_handle = NULL;
    if (Alpm::p_alpm == NULL || Alpm::p_alpm->m_alpm_handle == NULL) return;

    alpm_pkg_load(Alpm::p_alpm->m_alpm_handle,filename.toLocal8Bit().constData(),0,0,&m_handle);
    m_filepath = filename;
}

AlpmPackage::~AlpmPackage() {
    if (m_handle == NULL) return;
    if (type() == AlpmPackage::Type::File && m_delete) alpm_pkg_free(m_handle);
}

AlpmPackage::AlpmPackage(const AlpmPackage & pkg) {
    *this = pkg;
}

AlpmPackage::AlpmPackage(const AlpmPackage * pkg) {
    *this = *pkg;
}

int AlpmPackage::registerMetaType() {
    qRegisterMetaType<AlpmPackage>("AlpmPackage");
    qRegisterMetaType<QList<AlpmPackage> >("QList<AlpmPackage>");
    qRegisterMetaType<AlpmPackage::Dependence>("AlpmPackage::Dependence");
    qRegisterMetaType<AlpmPackage::FileInfo>("AlpmPackage::FileInfo");
    return 0;
}

QDataStream & operator<<(QDataStream &argument,const AlpmPackage::Dependence & dep) {
    argument << dep.name();
    argument << dep.version();
    argument << (int)dep.operation();
    argument << dep.description();

    return argument;
}

const QDataStream & operator>>(const QDataStream &argument,AlpmPackage::Dependence & dep) {
    (QDataStream &)argument >> dep.m_name;
    (QDataStream &)argument >> dep.m_version;
    int val;
    (QDataStream &)argument >> val;
    dep.m_operation = (AlpmPackage::CompareOper)val;
    (QDataStream &)argument >> dep.m_description;

    return argument;
}

QDataStream & operator<<(QDataStream &argument,const AlpmPackage::FileInfo & path) {
    argument << path.path();
    argument << (qint64)path.mode();
    argument << path.size();
    argument << path.date();

    return argument;
}

const QDataStream & operator>>(const QDataStream &argument,AlpmPackage::FileInfo & path) {
    (QDataStream &)argument >> path.m_path;
    qint64 val;
    (QDataStream &)argument >> val;
    path.m_mode = (mode_t)val;
    (QDataStream &)argument >> path.m_size;
    (QDataStream &)argument >> path.m_date;

    return argument;
}

QDataStream & operator<<(QDataStream &argument,const AlpmPackage & pkg) {
    argument << (qulonglong)pkg.handle();
    argument << QByteArray::fromRawData(pkg.name().latin1(),pkg.name().size());
    argument << QByteArray::fromRawData(pkg.version().latin1(),pkg.version().size());
    argument << QByteArray::fromRawData(pkg.description().latin1(),pkg.description().size());
    argument << QByteArray::fromRawData(pkg.repo().latin1(),pkg.repo().size());
    argument << pkg.isUpdate();
    argument << pkg.isInstalled();
    argument << pkg.filePath();
    return argument;
}

const QDataStream & operator>>(const QDataStream &argument,AlpmPackage & pkg) {
    qulonglong val;
    (QDataStream &)argument >> val;
    pkg.m_handle = (alpm_pkg_t *)val;
    (QDataStream &)argument >> pkg.m_filepath;
    (QDataStream &)argument >> pkg.m_delete;

    return argument;
}

bool AlpmPackage::isUpdate() const {
    if (repo() == "local") return false;
    alpm_pkg_t * pkg = findInLocal(m_handle);
    if (pkg == NULL) return false;
    int ret = alpm_pkg_vercmp(alpm_pkg_get_version(pkg),alpm_pkg_get_version(m_handle));
    int usage = 0;
    return ((ret < 0) && !alpm_db_get_usage(alpm_pkg_get_db(m_handle),&usage) && (usage & ALPM_DB_USAGE_UPGRADE) && !isIgnorable());
}

bool AlpmPackage::isIgnorable() const {
    return (m_handle == NULL) || (Alpm::instance() == NULL) || (Alpm::instance()->m_alpm_handle == NULL) || alpm_pkg_should_ignore(Alpm::p_alpm->m_alpm_handle,m_handle);
}

bool AlpmPackage::isValid() const {
    return ((type() != AlpmPackage::Type::Unknown) && (Alpm::p_alpm != NULL) && (Alpm::p_alpm->m_alpm_handle != NULL));
}

QLatin1String AlpmPackage::name() const {
    return QLatin1String((m_handle == NULL)?"":alpm_pkg_get_name(m_handle));
}

QLatin1String AlpmPackage::version() const {
    return QLatin1String((m_handle == NULL)?"":alpm_pkg_get_version(m_handle));
}

QLatin1String AlpmPackage::description() const {
    return QLatin1String((m_handle == NULL)?"":alpm_pkg_get_desc(m_handle));
}

QUrl AlpmPackage::url() const {
    if (m_handle == NULL) return QUrl();
    const char * str = alpm_pkg_get_url(m_handle);
    return (str == NULL)?QUrl():QUrl::fromEncoded(QByteArray::fromRawData(str,strlen(str)));
}

QString AlpmPackage::fileName() const {
    return (type() == AlpmPackage::Type::File)?QFileInfo(m_filepath).fileName():QString::fromLocal8Bit(alpm_pkg_get_filename(m_handle));
}

QString AlpmPackage::filePath() const {
    return m_filepath;
}

QList<QLatin1String> AlpmPackage::remoteLocations() const {
    QList<QLatin1String> m_remotelocs;
    if (m_handle == NULL) return m_remotelocs;
    alpm_db_t * db_t = alpm_pkg_get_db(m_handle);
    if (db_t == NULL) return m_remotelocs;
    AlpmList<char> locs(alpm_db_get_servers(db_t),AlpmList<char>::ignorefree);
    do {
         if (locs.isEmpty()) break;
         m_remotelocs.append(QLatin1String((const char *)locs.valuePtr()));
    } while(locs.next());
    locs.detach();

    return m_remotelocs;
}

bool AlpmPackage::isDownloaded(QString * path_pkg_file) const {
    if ((Alpm::p_alpm == NULL) || (Alpm::p_alpm->m_alpm_handle == NULL)) {
        if (Alpm::p_alpm != NULL) {
            Alpm::p_alpm->m_alpm_errno = Alpm::ALPM_IS_NOT_OPEN;
        }
        return false;
    }

    if (!m_filepath.isEmpty()) {
        *path_pkg_file = m_filepath;
        return true;
    }

    QStringList cacheDirs = Alpm::p_alpm->cacheDirs();
    if (cacheDirs.size() <= 0) return false;

    QString pkg_path;
    for (int i=0;i<cacheDirs.size();i++) {
        pkg_path = cacheDirs.at(i)+QDir::separator()+fileName();
        if (QFileInfo(pkg_path).exists()) {
            if (path_pkg_file != NULL) *path_pkg_file = pkg_path;
            return true;
        }
    }

    return false;
}

QDateTime AlpmPackage::buildDate() const {
    if (m_handle == NULL) return QDateTime();
    return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_builddate(m_handle));
}

QDateTime AlpmPackage::installDate() const {
    if (m_handle == NULL) return QDateTime();
    return QDateTime::fromSecsSinceEpoch(alpm_pkg_get_installdate(m_handle));
}

QString AlpmPackage::packager() const {
    return (m_handle == NULL)?QString():QString::fromLocal8Bit(alpm_pkg_get_packager(m_handle));
}

QLatin1String AlpmPackage::arch() const {
    return QLatin1String((m_handle == NULL)?"":alpm_pkg_get_arch(m_handle));
}

QStringList AlpmPackage::licenses() const {
    QStringList m_licenses;
    if (m_handle == NULL) return m_licenses;
    AlpmList<char> licenses(alpm_pkg_get_licenses(m_handle),AlpmList<char>::ignorefree);
    do {
         if (licenses.isEmpty()) break;
         m_licenses.append(QString::fromLocal8Bit((const char *)licenses.valuePtr()));
    } while(licenses.next());
    licenses.detach();
    return m_licenses;
}

QStringList AlpmPackage::groups() const {
    QStringList m_groups;
    if (m_handle == NULL) return m_groups;
    AlpmList<char> groups(alpm_pkg_get_groups(m_handle),AlpmList<char>::ignorefree);
    do {
         if (groups.isEmpty()) break;
         m_groups.append(QString::fromLocal8Bit((const char *)groups.valuePtr()));
    } while(groups.next());
    groups.detach();
    return m_groups;
}

QList<AlpmPackage::Dependence> AlpmPackage::depends() const {
    return alpm_pkg_list_processing((alpm_pkg_deplist_fn)alpm_pkg_get_depends,m_handle);
}

QList<AlpmPackage::Dependence> AlpmPackage::optdepends() const {
    return alpm_pkg_list_processing((alpm_pkg_deplist_fn)alpm_pkg_get_optdepends,m_handle);
}

QList<AlpmPackage::Dependence> AlpmPackage::conflicts() const {
    return alpm_pkg_list_processing((alpm_pkg_deplist_fn)alpm_pkg_get_conflicts,m_handle);
}

QList<AlpmPackage::Dependence> AlpmPackage::provides() const {
    return alpm_pkg_list_processing((alpm_pkg_deplist_fn)alpm_pkg_get_provides,m_handle);
}

QList<AlpmPackage::Dependence> AlpmPackage::replaces() const {
    return alpm_pkg_list_processing((alpm_pkg_deplist_fn)alpm_pkg_get_replaces,m_handle);
}

QStringList AlpmPackage::requiredby() const {
    QStringList m_requiredby;
    if ((Alpm::p_alpm == NULL) || (Alpm::p_alpm->m_alpm_handle == NULL) || (m_handle == NULL) || (type() == AlpmPackage::Type::File)) return m_requiredby;

    alpm_pkg_t * pkg = findInLocal(m_handle);
    if (pkg == NULL) pkg = m_handle;
    AlpmList<char> list(alpm_pkg_compute_requiredby(pkg));
    do {
        if (list.isEmpty()) break;
        m_requiredby.append(QString::fromLocal8Bit((const char *)list.valuePtr()));
    } while(list.next());

    return m_requiredby;
}

QStringList AlpmPackage::optionalfor() const {
    QStringList m_optionalfor;
    if ((Alpm::p_alpm == NULL) || (Alpm::p_alpm->m_alpm_handle == NULL) || (m_handle == NULL) || (type() == AlpmPackage::Type::File)) return m_optionalfor;

    alpm_pkg_t * pkg = findInLocal(m_handle);
    if (pkg == NULL) pkg = m_handle;
    AlpmList<char> list(alpm_pkg_compute_optionalfor(pkg));
    do {
        if (list.isEmpty()) break;
        m_optionalfor.append(QString::fromLocal8Bit((const char *)list.valuePtr()));
    } while(list.next());

    return m_optionalfor;
}

const QList<AlpmPackage::Dependence> AlpmPackage::alpm_pkg_list_processing(alpm_pkg_deplist_fn alpm_pkg_deplist,alpm_pkg_t * m_pkg) {
    AlpmPackage::Dependence dep;
    QList<AlpmPackage::Dependence> ret;
    AlpmList<alpm_depend_t> list(alpm_pkg_deplist(m_pkg),AlpmList<alpm_depend_t>::ignorefree);
    do {
        if (list.isEmpty()) break;
        dep.init(QString::fromLocal8Bit((const char *)list.valuePtr()->name),Dependence::mod_to_compareoper(list.valuePtr()->mod),QString::fromLocal8Bit((const char *)list.valuePtr()->version),QString::fromLocal8Bit((const char *)list.valuePtr()->desc));
        ret.append(dep);
    } while(list.next());
    list.detach();

    return ret;
}

AlpmPackage::Type AlpmPackage::type() const {
    if (m_handle == NULL) return AlpmPackage::Unknown;
    switch (alpm_pkg_get_origin(m_handle)) {
    case ALPM_PKG_FROM_LOCALDB:
        return AlpmPackage::Local;
    case ALPM_PKG_FROM_FILE:
        return AlpmPackage::File;
    case ALPM_PKG_FROM_SYNCDB:
        return AlpmPackage::SyncDB;
    default:
        break;
    }
    return AlpmPackage::Unknown;
}

QLatin1String AlpmPackage::repo() const {
    if (m_handle == NULL) return QLatin1String();
    alpm_db_t * db_t = alpm_pkg_get_db(m_handle);
    if (db_t == NULL || (type() == AlpmPackage::Type::File)) return QLatin1String();
    return QLatin1String(alpm_db_get_name(db_t));
}

qint64 AlpmPackage::size() const {
    return (qint64)alpm_pkg_get_size(m_handle);
}

qint64 AlpmPackage::installedSize() const {
    return (qint64)alpm_pkg_get_isize(m_handle);
}

AlpmPackage::Reason AlpmPackage::intToReason(alpm_pkgreason_t reason) const {
    switch (reason) {
    case ALPM_PKG_REASON_EXPLICIT:
        return AlpmPackage::Explicit;
    case ALPM_PKG_REASON_DEPEND:
        return AlpmPackage::Depend;
    default:
        break;
    }
    return AlpmPackage::Undefined;
}

alpm_pkgreason_t AlpmPackage::reasonToInt(AlpmPackage::Reason reason) const {
    switch (reason) {
    case AlpmPackage::Explicit:
        return ALPM_PKG_REASON_EXPLICIT;
    case AlpmPackage::Depend:
        return ALPM_PKG_REASON_DEPEND;
    default:
        break;
    }
    return ALPM_PKG_REASON_EXPLICIT;
}

AlpmPackage::Reason AlpmPackage::reason() const {
    if (repo() == "local") return intToReason(alpm_pkg_get_reason(m_handle));
    alpm_pkg_t * pkg = findInLocal(m_handle);
    if (pkg == NULL) return AlpmPackage::Undefined;
    return intToReason(alpm_pkg_get_reason(pkg));
}

bool AlpmPackage::setReason(Reason reason) {
    if (reason == AlpmPackage::Undefined) return false;
    if (repo() == "local") return (alpm_pkg_set_reason(m_handle,reasonToInt(reason)) == 0);
    alpm_pkg_t * pkg = findInLocal(m_handle);
    if (pkg == NULL) return false;

    return (alpm_pkg_set_reason(pkg,reasonToInt(reason)) == 0);
}

bool AlpmPackage::isFile() const {
    return (type() == AlpmPackage::Type::File);
}

AlpmPackage::FileInfo::FileInfo(const alpm_file_t & path) {
    m_path = "/"+QString::fromLocal8Bit((const char *)path.name);
    QFileInfo info(m_path);
    m_mode = toMode_t(info);
    m_size = (qint64)info.size();
    m_date = info.lastModified();
}

AlpmPackage::FileInfo::FileInfo(const QString & path,qint64 size,mode_t mode,const QDateTime & date) {
    m_path = "/"+path;
    m_mode = mode;
    m_size = size;
    m_date = date;
}

AlpmPackage::FileInfo::FileInfo() {
    m_mode = 0;
    m_size = 0;
}

mode_t AlpmPackage::FileInfo::toMode_t(const QFileInfo & info) {
    mode_t mode = 0;
    QFile::Permissions permissions = info.permissions();
    if (permissions & (QFile::ReadOwner | QFile::ReadUser))
        mode |= S_IRUSR;
    if (permissions & (QFile::WriteOwner | QFile::WriteUser))
        mode |= S_IWUSR;
    if (permissions & (QFile::ExeOwner | QFile::ExeUser))
        mode |= S_IXUSR;
    if (permissions & QFile::ReadGroup)
        mode |= S_IRGRP;
    if (permissions & QFile::WriteGroup)
        mode |= S_IWGRP;
    if (permissions & QFile::ExeGroup)
        mode |= S_IXGRP;
    if (permissions & QFile::ReadOther)
        mode |= S_IROTH;
    if (permissions & QFile::WriteOther)
        mode |= S_IWOTH;
    if (permissions & QFile::ExeOther)
        mode |= S_IXOTH;
    if (info.isSymbolicLink()) mode |= S_IFLNK;
    else if (info.isFile()) mode |= S_IFREG;
    else if (info.isDir()) mode |= S_IFDIR;
    return mode;
}

QString AlpmPackage::FileInfo::path() const {
    return m_path;
}

qint64 AlpmPackage::FileInfo::size() const {
    return m_size;
}

mode_t AlpmPackage::FileInfo::mode() const {
    return m_mode;
}

QDateTime AlpmPackage::FileInfo::date() const {
    return m_date;
}

QList<AlpmPackage::FileInfo> AlpmPackage::files(const QString & archive_path) const {
    QList<AlpmPackage::FileInfo> ret;

    for(ArchiveEntry * inst : ArchiveFileReader(archive_path)) {
        ret.append(AlpmPackage::FileInfo(inst->entryName(),inst->entrySize(),inst->entryPerm(),inst->entryModificationDate()));
    }
    return ret;
}

QList<AlpmPackage::FileInfo> AlpmPackage::files(alpm_pkg_t * pkg) const {
    QList<AlpmPackage::FileInfo> ret;
    if (pkg == NULL) return ret;

    alpm_filelist_t * files = alpm_pkg_get_files(pkg);
    if (files == NULL) return ret;

    for (size_t i=0;i<files->count;i++) {
        ret.append(AlpmPackage::FileInfo(files->files[i]));
    }
    return ret;
}

QList<AlpmPackage::FileInfo> AlpmPackage::files() const {
    if (type() == AlpmPackage::Type::File) return files(m_filepath);
    else {
        if ((Alpm::p_alpm == NULL) || (Alpm::p_alpm->m_alpm_handle == NULL)) {
            if (Alpm::p_alpm != NULL) {
                Alpm::p_alpm->m_alpm_errno = Alpm::ALPM_IS_NOT_OPEN;
            }
            return QList<AlpmPackage::FileInfo>();
        }

        alpm_pkg_t * m_pkg = handle();
        QList<AlpmPackage::FileInfo> ret = files(m_pkg);
        if (!ret.isEmpty()) return ret;
        if (repo() != "local") m_pkg = findInLocal(m_pkg);
        if (m_pkg != NULL) return files(m_pkg);
        else {
            for (QString & dir: Alpm::instance()->cacheDirs()) {
                QString filepath = dir+fileName();
                if (QFile(filepath).exists()) return files(filepath);
            }
        }

        return ret;
    }
}

bool AlpmPackage::hasFile(const QString & path) const {
    for (AlpmPackage::FileInfo & file: files()) {
        if (file.path() == path) return true;
    }
    return false;
}

bool AlpmPackage::hasFileAs(const QRegularExpression & expr) const {
    for (AlpmPackage::FileInfo & file: files()) {
        if (expr.match(file.path()).hasMatch()) return true;
    }
    return false;
}

bool AlpmPackage::hasProviderAs(const QRegularExpression & expr) const {
    for (Dependence & provide: provides()) {
        if (expr.match(provide.name()).hasMatch()) return true;
    }
    return false;
}

bool AlpmPackage::hasDescriptionAs(const QRegularExpression & expr) const {
    return expr.match(description()).hasMatch();
}

bool AlpmPackage::hasNameAs(const QRegularExpression & expr) const {
    return expr.match(name()).hasMatch();
}

bool AlpmPackage::isOrphaned() const {
    return (isInstalled() && (reason() != Explicit) && requiredby().isEmpty() && optionalfor().isEmpty());
}

alpm_pkg_t * AlpmPackage::findInLocal(alpm_pkg_t * pkg) const {
    if ((Alpm::instance() == NULL) || (Alpm::instance()->m_alpm_handle == NULL)) return NULL;
    return Alpm::instance()->localDB().findByPackageName(alpm_pkg_get_name(pkg)).handle();
}

int AlpmPackage::pkg_version_compare(alpm_pkg_t * item1, alpm_pkg_t * item2) {
    int ret = strcmp(alpm_pkg_get_name(item1),alpm_pkg_get_name(item2));
    if (ret != 0) return ret;
    return strcmp(alpm_pkg_get_version(item1),alpm_pkg_get_version(item2));
}

alpm_pkg_t * AlpmPackage::findInSync(alpm_pkg_t * pkg) const {
    if ((Alpm::instance() == NULL) || (Alpm::instance()->m_alpm_handle == NULL)) return NULL;

    alpm_pkg_t * ret = NULL;
    const char * repo = alpm_db_get_name(alpm_pkg_get_db(pkg));

    for (AlpmDB & db: Alpm::instance()->allSyncDBs()) {
        if (db.name() == repo) continue;
        ret = alpm_db_get_pkg(db.m_db_handle,alpm_pkg_get_name(pkg));
        if (alpm_pkg_vercmp(alpm_pkg_get_version(pkg),alpm_pkg_get_version(ret))) continue;
        break;
    }

    return ret;
}

bool AlpmPackage::isInstalled() const {
    if (repo() == "local") return true;
    alpm_pkg_t * pkg = findInLocal(m_handle);
    if (pkg == NULL) return false;
    return (alpm_pkg_vercmp(alpm_pkg_get_version(pkg),alpm_pkg_get_version(m_handle)) == 0);
}

bool AlpmPackage::splitname_ver(const QString & target,QString & name,QString & version) {
    int index = target.lastIndexOf('-');
    if (index == -1) return false;

    bool ok;
    target.mid(index+1).toDouble(&ok);
    if (!ok) return false;

    index = target.lastIndexOf('-',index-1);
    if (index == -1) return false;
    version = target.mid(index+1);
    name = target.mid(0,index);

    return true;
}

AlpmPackage::CompareOper AlpmPackage::parseNameVersion(const QString & str,QString & name,QString & ver) {
    QString repo;
    return parseNameVersion(str,repo,name,ver);
}

AlpmPackage::CompareOper AlpmPackage::parseNameVersion(const QString & str,QString & repo,QString & name,QString & ver) {
    int count = 2;

    repo.clear();
    int slash_index = str.indexOf('/');
    if (slash_index > 0) repo = str.mid(0,slash_index);

    AlpmPackage::CompareOper oper = UNKNOWN;
    int index = str.indexOf(">=");
    if (index == -1) {
        index = str.indexOf("<=");
        if (index == -1) {
            index = str.indexOf("=");
            if (index == -1) {
                index = str.indexOf(">");
                if (index == -1) {
                    index = str.indexOf("<");
                    if (index == -1) {
                        if (splitname_ver(str.mid(slash_index+1),name,ver)) return EQUAL;
                        else {
                            name = str.mid(slash_index+1).simplified();
                            ver = "";
                            return oper;
                        }
                    }
                    else {
                        count = 1;
                        oper = LESS;
                    }
                }
                else {
                    count = 1;
                    oper = MORE;
                }
            }
            else {
                count = 1;
                oper = EQUAL;
            }
        }
        else oper = LESS_OR_EQUAL;
    }
    else oper = MORE_OR_EQUAL;

    name = str.mid(slash_index+1,index-slash_index-1).simplified();
    ver = str.mid(index+count).simplified();

    return oper;
}

QString AlpmPackage::toString() const {
    return name()+"="+version();
}

bool AlpmPackage::containsText(const QString & text,SearchFieldType field) {
    if (text.isEmpty()) return true;

    switch (field) {
    case NAME:
        return name().contains(text,Qt::CaseInsensitive);
    case PROVIDER:
        for (Dependence & provide: provides()) {
            if (provide.isAppropriate(Dependence::fromString(text))) return true;
        }
        return false;
    case DESC:
        return description().contains(text,Qt::CaseInsensitive);
    case DEPENDENCY:
        for (Dependence & dependency: depends()) {
            if (dependency.isAppropriate(Dependence::fromString(text))) return true;
        }
        return false;
    default:
        break;
    }

    return false;
}

AlpmPackage & AlpmPackage::operator=(const AlpmPackage &other) {
    if (other.type() == AlpmPackage::Type::File && other.m_delete) {
        m_handle = NULL;
        alpm_pkg_load(Alpm::p_alpm->m_alpm_handle,other.filePath().toLocal8Bit().constData(),1,0,&m_handle);
        m_filepath = other.filePath();
        m_delete = true;
    }
    else {
        m_handle = other.m_handle;
        m_filepath = other.filePath();
        m_delete = other.m_delete;
    }
    return *this;
}

bool AlpmPackage::operator==(const AlpmPackage &other) {
    return (name() == other.name() && version() == other.version() && repo() == other.repo());
}

bool AlpmPackage::ownedByGroup(const QString & group) {
    return groups().contains(group);
}

QString AlpmPackage::download() {
    QStringList paths = Alpm::p_alpm->download_packages(QList<AlpmPackage>() << *this);
    if (paths.count() <= 0) return QString();

    return paths.at(0);
}

alpm_pkg_t * AlpmPackage::handle() const {
    return m_handle;
}

QList<AlpmPackage::UserChangeStatus> AlpmPackage::possibleChangeStatuses() const {
    return possibleChangeStatuses(*this);
}

const QList<AlpmPackage::UserChangeStatus> AlpmPackage::possibleChangeStatuses(const AlpmPackage & pkg) {
    if (pkg.isInstalled()) {
        if ((pkg.repo() == "local" || pkg.repo().isEmpty() || pkg.repo().isNull())) return QList<AlpmPackage::UserChangeStatus>() << DO_UNINSTALL_ALL << DO_UNINSTALL;
        else return QList<AlpmPackage::UserChangeStatus>() << DO_REINSTALL << DO_REINSTALL_ASDEPS << DO_UNINSTALL_ALL << DO_UNINSTALL;
    }
    return QList<AlpmPackage::UserChangeStatus>() << DO_INSTALL << DO_INSTALL_ASDEPS << DO_INSTALL_FORCE << DO_INSTALL_ASDEPS_FORCE;
}

AlpmPackage::UserChangeStatus AlpmPackage::defaultStatus() const {
    return isInstalled()?DO_UNINSTALL_ALL:DO_INSTALL;
}

AlpmPackage::UserChangeStatus AlpmPackage::changeStatus() const {
    return m_change_statuses.contains(handle())?m_change_statuses[handle()]:AlpmPackage::DO_NOTHING;
}

bool AlpmPackage::setChangeStatus(const AlpmPackage & pkg,UserChangeStatus status) {
    if (status == DO_NOTHING) m_change_statuses.remove(pkg.handle());
    else {
        if (!possibleChangeStatuses(pkg).contains(status)) return false;
        m_change_statuses[pkg.handle()] = status;
    }
    return true;
}

int AlpmPackage::pkg_vercmp(const QString & ver1, const QString & ver2) {
    return ::alpm_pkg_vercmp(ver1.toLatin1().constData(),ver2.toLatin1().constData());
}

QUrl AlpmPackage::iconUrl() const {
    if (name() == "qpacman") return QUrl("qrc://pics/qpacman.svg");

    if (m_pool == NULL) {
        m_pool = new AppStream::Pool(qApp);
        if (!m_pool->load()) {
            qWarning() << "Unable to open AppStream pool:" << m_pool->lastError();
            return QUrl();
        }

        const QList<AppStream::Component> apps = m_pool->componentsByKind(AppStream::Component::KindDesktopApp);
        for (const AppStream::Component &app : apps) {
            const QStringList pkgNames = app.packageNames();
            for (const QString &pkgName : pkgNames) {
                m_appInfo.insert(pkgName, app);
            }
        }
    }

    QUrl url = icon_url(name());
    if (url.isEmpty()) {
        for (Dependence & dep: provides()) {
            url = icon_url(dep.name());
            if (!url.isEmpty()) break;
        }
    }

    return url;
}

QUrl AlpmPackage::icon_url(const QString & name) const {
    QUrl url64;
    QUrl url;
    if (m_appInfo.contains(name)) {
        AppStream::Component app = m_appInfo.value(name);
        const QList<AppStream::Icon> icons = app.icons();
        for (const AppStream::Icon &icon : icons) {
            if (icon.isEmpty()) continue;
            if (icon.width() < 64 && url.isEmpty()) url = icon.url();
            else if (url64.isEmpty()) url64 = icon.url();
        }
    }

    return url64.isEmpty()?url:url64;
}

const QList<AlpmPackage> AlpmPackage::changedStatusPackages() {
    QList<AlpmPackage> ret;
    QMapIterator<alpm_pkg_t_p,AlpmPackage::UserChangeStatus> i(m_change_statuses);
    while (i.hasNext()) {
        i.next();
        ret.append(AlpmPackage(i.key().ptr()));
    }
    return ret;
}

void AlpmPackage::resetAllChangeStatuses() {
    m_change_statuses.clear();
}

bool operator<(const AlpmPackage::alpm_pkg_t_p & e1, const AlpmPackage::alpm_pkg_t_p & e2) {
    int ret = strcmp(alpm_pkg_get_name(e1.ptr()),alpm_pkg_get_name(e2.ptr()));
    if (ret != 0) return (ret < 0);
    return alpm_pkg_vercmp(alpm_pkg_get_version(e1.ptr()),alpm_pkg_get_version(e2.ptr()));
}