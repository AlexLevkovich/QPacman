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
#include <AppStreamQt/pool.h>
#include <AppStreamQt/image.h>
#include <AppStreamQt/icon.h>
#include <QDebug>
#include <limits>
#include "archivefilesiterator.h"

AppStream::Pool * AlpmPackage::m_pool = NULL;
QMultiHash<QString, AppStream::Component> AlpmPackage::m_appInfo;

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

AlpmPackage::Dependence::Dependence(const QString & name,CompareOper operation,const QString & version,const QString & description) {
    init(name,operation,version,description);
}
AlpmPackage::Dependence::Dependence(AlpmPackage * pkg,CompareOper operation) {
    init(pkg,operation);
}

void AlpmPackage::Dependence::init(const QString & name,CompareOper operation,const QString & version,const QString & description) {
    m_name = name;
    m_version = version;
    m_description = description;
    m_operation = operation;
    if (m_operation == UNKNOWN) m_version.clear();
    m_installed = 'N';
}

void AlpmPackage::Dependence::init(AlpmPackage * pkg,CompareOper operation) {
    init(pkg->name(),operation,pkg->version());
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
        this->mod = (alpm_depmod_t)mod;
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
        this->mod = (alpm_depmod_t)dep.operation();
        this->name_hash = hash_from_name();
    }

private:
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

bool AlpmPackage::Dependence::isAppropriate(const AlpmPackage * pkg) const {
    if (pkg == NULL) return false;

    return isAppropriate(pkg->name(),pkg->version());
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

    QVector<AlpmPackage *> pkgs = version().isEmpty()?alpm->findByPackageName(name()):alpm->findByPackageNameVersion(name(),version());
    if (pkgs.count() <= 0) {
        pkgs = alpm->findByPackageNameProvides(*this);
        if (pkgs.count() <= 0) return ret;
    }

    for (int i=0;i<pkgs.count();i++) {
        if (pkgs[i]->isInstalled()) return pkgs[i]->depends();
    }
    return pkgs[((int)provider_id >= pkgs.count())?(pkgs.count()-1):(int)provider_id]->depends();
}

bool AlpmPackage::Dependence::isInstalled() const {
    if (m_installed != 'N') return (m_installed == 'I');

    Alpm * alpm = Alpm::instance();
    if (alpm == NULL) return false;

    AlpmDB db = alpm->localDB();
    if (db.findByPackageName(name()).count() > 0) return true;
    bool ret = (db.findCacheIndexesByPackageNameProvides(*this).count() > 0);
    ((AlpmPackage::Dependence *)this)->m_installed = ret?'I':'U';
    return ret;
}

QString AlpmPackage::Dependence::urlParms() const {
    if (operation() == AlpmPackage::UNKNOWN) return description().isEmpty()?name():(name()+"?descr="+description());
    return name()+QString("?oper=%1&ver=%2%3").arg(operation()).arg(version()).arg(description().isEmpty()?"":"&descr="+description());
}

QUrl AlpmPackage::Dependence::url(const QString & type) const {
    return QString("qpc://%1/%2").arg(type).arg(urlParms());
}

AlpmPackage::AlpmPackage() {
    m_type = AlpmPackage::Type::Unknown;
    init(NULL);
}

AlpmPackage::AlpmPackage(alpm_pkg_t *pkg) {
    m_type = AlpmPackage::Type::Unknown;
    init(pkg);
}

AlpmPackage::AlpmPackage(const QString & name,const QString & version) {
    m_type = AlpmPackage::Type::Unknown;
    init(NULL);
    m_version = version;
    m_name = name;
}

AlpmPackage::AlpmPackage(const QString & filename) {
    m_type = AlpmPackage::Type::Unknown;
    if (Alpm::p_alpm == NULL || Alpm::p_alpm->m_alpm_handle == NULL) return;

    alpm_pkg_t * m_handle = NULL;
    alpm_pkg_load(Alpm::p_alpm->m_alpm_handle,filename.toLocal8Bit().constData(),1,0,&m_handle);
    init(m_handle);
    if (m_handle == NULL) return;
    alpm_pkg_free(m_handle);

    m_filepath = filename;
}

AlpmPackage::AlpmPackage(const AlpmPackage & pkg) {
    m_type = AlpmPackage::Type::Unknown;
    *this = pkg;
}

AlpmPackage::AlpmPackage(const AlpmPackage * pkg) {
    m_type = AlpmPackage::Type::Unknown;
    *this = *pkg;
}

void AlpmPackage::init(alpm_pkg_t * m_pkg) {
    setInstalled(false);
    m_isChosen = false;
    m_isUpdate = false;
    m_isIgnorable = true;
    m_reason = Undefined;
    m_pkg_size = 0;
    m_pkg_isize = 0;
    if (m_pkg != NULL) {
        m_name = QString::fromLatin1(alpm_pkg_get_name(m_pkg));
        m_version = QString::fromLatin1(alpm_pkg_get_version(m_pkg));
        m_desc = QString::fromLocal8Bit(alpm_pkg_get_desc(m_pkg));
        const char * str = alpm_pkg_get_url(m_pkg);
        m_url = (str == NULL)?QUrl():QUrl::fromEncoded(QByteArray::fromRawData(str,strlen(str)));
        m_arch = QString::fromLatin1(alpm_pkg_get_arch(m_pkg));
        str = alpm_pkg_get_filename(m_pkg);
        m_filename = (str == NULL)?(name()+"-"+version()+"-"+arch()+".pkg.tar.xz"):QString::fromLatin1(str);
        QDateTime date;
        date.setSecsSinceEpoch(alpm_pkg_get_builddate(m_pkg));
        m_build_date = date;
        date.setSecsSinceEpoch(alpm_pkg_get_installdate(m_pkg));
        m_install_date = date;
        m_packager = QString::fromLocal8Bit(alpm_pkg_get_packager(m_pkg));
        AlpmList<char> licenses(alpm_pkg_get_licenses(m_pkg),AlpmList<char>::ignorefree);
        do {
             if (licenses.isEmpty()) break;
             m_licenses.append(QString::fromLocal8Bit((const char *)licenses.valuePtr()));
        } while(licenses.next());
        licenses.detach();
        AlpmList<char> groups(alpm_pkg_get_groups(m_pkg),AlpmList<char>::ignorefree);
        do {
             if (groups.isEmpty()) break;
             m_groups.append(QString::fromLocal8Bit((const char *)groups.valuePtr()));
        } while(groups.next());
        groups.detach();
        m_depends = alpm_pkg_list_processing((alpm_pkg_deplist_fn)alpm_pkg_get_depends,m_pkg);
        m_optdepends = alpm_pkg_list_processing((alpm_pkg_deplist_fn)alpm_pkg_get_optdepends,m_pkg);
        m_conflicts = alpm_pkg_list_processing((alpm_pkg_deplist_fn)alpm_pkg_get_conflicts,m_pkg);
        m_provides = alpm_pkg_list_processing((alpm_pkg_deplist_fn)alpm_pkg_get_provides,m_pkg);
        m_replaces = alpm_pkg_list_processing((alpm_pkg_deplist_fn)alpm_pkg_get_replaces,m_pkg);
        m_type = m_filepath.isEmpty()?(AlpmPackage::Type)alpm_pkg_get_origin(m_pkg):AlpmPackage::Type::File;
        m_pkg_size = alpm_pkg_get_size(m_pkg);
        m_pkg_isize = alpm_pkg_get_isize(m_pkg);

        alpm_db_t * db_t = alpm_pkg_get_db(m_pkg);
        if (db_t == NULL || (type() == AlpmPackage::Type::File)) {
            setInstalled(false);
            m_dbname = QString::fromLatin1("aur");
        }
        else {
            m_dbname = QString::fromLocal8Bit(alpm_db_get_name(db_t));
            AlpmList<char> locs(alpm_db_get_servers(db_t),AlpmList<char>::ignorefree);
            do {
                 if (locs.isEmpty()) break;
                 m_remotelocs.append(QString::fromLocal8Bit((const char *)locs.valuePtr()));
            } while(locs.next());
            locs.detach();

            if (m_dbname == QString::fromLatin1("local")) setInstalled(true);
            else {
                m_isIgnorable = (Alpm::p_alpm->m_alpm_handle == NULL) || alpm_pkg_should_ignore(Alpm::p_alpm->m_alpm_handle,m_pkg);
                alpm_pkg_t * pkg = alpm_db_get_pkg(Alpm::p_alpm->localDB().m_db_handle,alpm_pkg_get_name(m_pkg));
                if (pkg == NULL) setInstalled(false);
                else {
                    int ret;
                    if (!(ret = alpm_pkg_vercmp(alpm_pkg_get_version(pkg),alpm_pkg_get_version(m_pkg)))) setInstalled(true);
                    else {
                        setInstalled(false);
                        int usage = 0;
                        m_isUpdate = ((ret < 0) && !alpm_db_get_usage(db_t,&usage) && (usage & ALPM_DB_USAGE_UPGRADE) && !m_isIgnorable);
                    }
                }

                if (m_installed) {
                    AlpmList<char> list(alpm_pkg_compute_requiredby(pkg));
                    do {
                        if (list.isEmpty()) break;
                        m_requiredby.append(QString::fromLocal8Bit((const char *)list.valuePtr()));
                    } while(list.next());
                }
            }
        }
    }
}

bool AlpmPackage::isValid() const {
    return ((type() != AlpmPackage::Type::Unknown) && (Alpm::p_alpm != NULL) && (Alpm::p_alpm->m_alpm_handle != NULL));
}

QString AlpmPackage::name() const {
    return m_name;
}

QString AlpmPackage::version() const {
    return m_version;
}

QString AlpmPackage::description() const {
    return m_desc;
}

QUrl AlpmPackage::url() const {
    return m_url;
}

QString AlpmPackage::fileName() const {
    return m_filename;
}

QString AlpmPackage::filePath() const {
    return m_filepath;
}

QStringList AlpmPackage::remoteLocations() const {
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
    return m_build_date;
}

QDateTime AlpmPackage::installDate() const {
    return m_install_date;
}

QString AlpmPackage::packager() const {
    return m_packager;
}

QString AlpmPackage::arch() const {
    return m_arch;
}

QStringList AlpmPackage::licenses() const {
    return m_licenses;
}

QStringList AlpmPackage::groups() const {
    return m_groups;
}

QList<AlpmPackage::Dependence> AlpmPackage::depends() const {
    return m_depends;
}

/*QList<AlpmPackage::Dependence> AlpmPackage::resolve(const QList<AlpmPackage *> & additional_deps) const {
    QList<AlpmPackage::Dependence> ret;
    if (isInstalled()) return ret;

    bool appropriate;
    for (AlpmPackage::Dependence dep: depends()) {
        appropriate = false;
        for (AlpmPackage * pkg: additional_deps) {
            if (dep.isAppropriate(pkg)) {
                appropriate = true;
                ret.append(dep);
                break;
            }
        }
        if (!appropriate) resolve_portion(dep,ret);
    }

    std::sort(ret.begin(),ret.end());
    size_t index = 0;
    ret.erase(std::remove_if(ret.begin(),ret.end(),[&ret,&index](const AlpmPackage::Dependence & dep) {
        index++;
        if (index == 1) return false;
        return ret[index-2].isAppropriate(dep);
    }),ret.end());

    return ret;
}

void AlpmPackage::resolve_portion(const AlpmPackage::Dependence & dep,QList<AlpmPackage::Dependence> & list) const {
    if (dep.isInstalled()) return;
    qDebug() << dep.name() << dep.version();
    list.append(dep);
    for (AlpmPackage::Dependence subdep: dep.findDepends()) {
        resolve_portion(subdep,list);
    }
}*/

QList<AlpmPackage::Dependence> AlpmPackage::optdepends() const {
    return m_optdepends;
}

QList<AlpmPackage::Dependence> AlpmPackage::conflicts() const {
    return m_conflicts;
}

QList<AlpmPackage::Dependence> AlpmPackage::provides() const {
    return m_provides;
}

QStringList AlpmPackage::requiredby() const {
    if (!m_requiredby.isEmpty()) return m_requiredby;
    if (type() == AlpmPackage::Type::File) return m_requiredby;

    AlpmPackage * p_this = (AlpmPackage *)this;
    alpm_pkg_t * handle = p_this->handle();
    if ((Alpm::p_alpm == NULL) || (Alpm::p_alpm->m_alpm_handle == NULL) || (handle == NULL)) return m_requiredby;

    AlpmList<char> list(alpm_pkg_compute_requiredby(handle));
    do {
        if (list.isEmpty()) break;
        p_this->m_requiredby.append(QString::fromLocal8Bit((const char *)list.valuePtr()));
    } while(list.next());

    return m_requiredby;
}

QStringList AlpmPackage::optionalfor() const {
    if (!m_optionalfor.isEmpty()) return m_optionalfor;
    if (type() == AlpmPackage::Type::File) return m_optionalfor;

    AlpmPackage * p_this = (AlpmPackage *)this;
    alpm_pkg_t * handle = p_this->handle();
    if ((Alpm::p_alpm == NULL) || (Alpm::p_alpm->m_alpm_handle == NULL) || (handle == NULL)) return m_optionalfor;

    AlpmList<char> list2(alpm_pkg_compute_optionalfor(handle));
    do {
        if (list2.isEmpty()) break;
        p_this->m_optionalfor.append(QString::fromLocal8Bit((const char *)list2.valuePtr()));
    } while(list2.next());

    return m_optionalfor;
}

QList<AlpmPackage::Dependence> AlpmPackage::replaces() const {
    return m_replaces;
}

const QList<AlpmPackage::Dependence> AlpmPackage::alpm_pkg_list_processing(alpm_pkg_deplist_fn alpm_pkg_deplist,alpm_pkg_t * m_pkg) {
    AlpmPackage::Dependence dep;
    QList<AlpmPackage::Dependence> ret;
    AlpmList<alpm_depend_t> list(alpm_pkg_deplist(m_pkg),AlpmList<alpm_depend_t>::ignorefree);
    do {
        if (list.isEmpty()) break;
        dep.init(QString::fromLocal8Bit((const char *)list.valuePtr()->name),(CompareOper)list.valuePtr()->mod,QString::fromLocal8Bit((const char *)list.valuePtr()->version),QString::fromLocal8Bit((const char *)list.valuePtr()->desc));
        ret.append(dep);
    } while(list.next());
    list.detach();

    return ret;
}

AlpmPackage::Type AlpmPackage::type() const {
    return m_type;
}

QString AlpmPackage::repo() const {
    return m_dbname;
}

off_t AlpmPackage::size() const {
    return m_pkg_size;
}

off_t AlpmPackage::installedSize() const {
    return m_pkg_isize;
}

AlpmPackage::Reason AlpmPackage::reason() const {
    if (m_reason == AlpmPackage::Undefined) {
        alpm_db_t * db_t = alpm_get_localdb(Alpm::p_alpm->m_alpm_handle);
        if (db_t == NULL) return m_reason;
        alpm_pkg_t * pkg = alpm_db_get_pkg(db_t,name().toLocal8Bit().constData());
        if (pkg != NULL) ((AlpmPackage *)this)->m_reason = (AlpmPackage::Reason)alpm_pkg_get_reason(pkg);
    }
    return m_reason;
}

bool AlpmPackage::setReason(Reason reason) {
    if (type() == AlpmPackage::Type::File) return false;

    if ((Alpm::p_alpm == NULL) || (Alpm::p_alpm->m_alpm_handle == NULL)) {
        if (Alpm::p_alpm != NULL) {
            Alpm::p_alpm->m_alpm_errno = Alpm::ALPM_IS_NOT_OPEN;
        }
        return false;
    }

    alpm_db_t * db_t = alpm_get_localdb(Alpm::p_alpm->m_alpm_handle);
    if (db_t == NULL) return false;
    alpm_pkg_t * pkg = alpm_db_get_pkg(db_t,name().toLocal8Bit().constData());
    if (pkg != NULL) {
        int ret = alpm_pkg_set_reason(pkg,(alpm_pkgreason_t)reason);
        if (ret == 0) m_reason = reason;
        return (ret == 0);
    }
    Alpm::p_alpm->m_alpm_errno=Alpm::REASON_WRONG_DB;
    return false;
}

bool AlpmPackage::isFile() const {
    return (type() == AlpmPackage::Type::File);
}

QStringList AlpmPackage::files() const {
    if (type() == AlpmPackage::Type::File) return PackageFileReader::fileList(m_filepath,true);
    else {
        if ((Alpm::p_alpm == NULL) || (Alpm::p_alpm->m_alpm_handle == NULL)) {
            if (Alpm::p_alpm != NULL) {
                Alpm::p_alpm->m_alpm_errno = Alpm::ALPM_IS_NOT_OPEN;
            }
            return QStringList();
        }

        alpm_db_t * db_t = NULL;
        alpm_pkg_t * pkg = NULL;
        if (isInstalled()) {
            db_t = alpm_get_localdb(Alpm::p_alpm->m_alpm_handle);
        }
        else {
            pkg  = handle();
            if (pkg != NULL) {
                db_t = alpm_pkg_get_db(pkg);
            }
        }

        if (db_t == NULL) return QStringList();
        pkg = (pkg == NULL)?alpm_db_get_pkg(db_t,name().toLocal8Bit().constData()):pkg;

        if (pkg != NULL) {
            QStringList ret;
            alpm_filelist_t * files = alpm_pkg_get_files(pkg);
            if (files == NULL) return ret;

            for (size_t i=0;i<files->count;i++) {
                ret.append("/"+QString::fromLocal8Bit((const char *)files->files[i].name));
            }

            return ret;
        }

    }

    return QStringList();
}

bool AlpmPackage::hasFileAs(const QRegularExpression & expr) const {
    QStringList filelist = files();
    for (int i=0;i<filelist.count();i++) {
        if (expr.match(filelist.at(i)).hasMatch()) return true;
    }
    return false;
}

bool AlpmPackage::hasProviderAs(const QRegularExpression & expr) const {
    for (int i=0;i<m_provides.count();i++) {
        if (expr.match(m_provides.at(i).name()).hasMatch()) return true;
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
    return (isInstalled() && (reason() != Explicit) && requiredby().isEmpty());
}

bool AlpmPackage::isInstalled() const {
    return (type() == Local)?true:m_installed;
}

void AlpmPackage::setInstalled(bool installed) {
    m_installed = (type() == Local)?true:installed;

    if (!m_installed) m_change_status = DO_INSTALL;
    else m_change_status = DO_UNINSTALL_ALL;
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
        for (Dependence provide: m_provides) {
            if (provide.isAppropriate(Dependence::fromString(text))) return true;
        }
        return false;
    case DESC:
        return description().contains(text,Qt::CaseInsensitive);
    case DEPENDENCY:
        for (Dependence dependency: m_depends) {
            if (dependency.isAppropriate(Dependence::fromString(text))) return true;
        }
        return false;
    default:
        break;
    }

    return false;
}

AlpmPackage & AlpmPackage::operator=(const AlpmPackage &other) {
    m_name = other.m_name;
    m_version = other.m_version;
    m_desc = other.m_desc;
    m_groups = other.m_groups;
    m_provides = other.m_provides;
    m_dbname = other.m_dbname;
    m_installed = other.m_installed;
    m_change_status = other.m_change_status;
    m_reason = other.m_reason;
    m_filepath = other.m_filepath;
    m_isChosen = other.m_isChosen;
    m_isUpdate = other.m_isUpdate;
    m_isIgnorable = other.m_isIgnorable;
    m_url = other.m_url;
    m_filename = other.m_filename;
    m_arch = other.m_arch;
    m_remotelocs = other.m_remotelocs;
    m_build_date = other.m_build_date;
    m_install_date = other.m_install_date;
    m_packager = other.m_packager;
    m_licenses = other.m_licenses;
    m_depends = other.m_depends;
    m_optdepends = other.m_optdepends;
    m_requiredby = other.m_requiredby;
    m_conflicts = other.m_conflicts;
    m_replaces = other.m_replaces;
    m_optionalfor = other.m_optionalfor;
    m_type = other.m_type;
    m_pkg_size = other.m_pkg_size;
    m_pkg_isize = other.m_pkg_isize;
    return *this;
}

bool AlpmPackage::operator==(const AlpmPackage &other) {
    return (name() == other.name() && version() == other.version() && repo() == other.repo());
}

bool AlpmPackage::ownedByGroup(const QString & group) {
    return groups().contains(group);
}

QString AlpmPackage::download() {
    QStringList paths;

    if (Alpm::p_alpm->downloadPackages(QList<AlpmPackage *>() << this,&paths) != ThreadRun::OK) return QString();
    if (paths.count() <= 0) return QString();

    return paths.at(0);
}

alpm_pkg_t * AlpmPackage::handle() const {
    if (Alpm::instance() == NULL || Alpm::instance()->m_alpm_handle == NULL) return NULL;

    if (type() == AlpmPackage::Type::File) {
        alpm_pkg_t * m_handle;
        alpm_pkg_load(Alpm::instance()->m_alpm_handle,m_filepath.toLocal8Bit().constData(),1,0,&m_handle);
        return m_handle;
    }

    if (m_type == Local) {
        alpm_db_t * db_t = alpm_get_localdb(Alpm::instance()->m_alpm_handle);
        if (db_t == NULL) return NULL;
        alpm_pkg_t * pkg = alpm_db_get_pkg(db_t,name().toLocal8Bit().constData());
        if (pkg == NULL) return NULL;
        if (QString::fromLatin1(alpm_pkg_get_version(pkg)) == version()) {
            return pkg;
        }
        return NULL;
    }

    if (m_type == SyncDB) {
        alpm_pkg_t * pkg = NULL;
        char * db_name;
        AlpmList<alpm_db_t> dbs(alpm_get_syncdbs(Alpm::instance()->m_alpm_handle),AlpmList<alpm_db_t>::ignorefree);
        do {
            if (dbs.isEmpty()) break;
            pkg = alpm_db_get_pkg(dbs.valuePtr(),name().toLocal8Bit().constData());
            if (pkg == NULL) continue;
            if ((QString::fromLatin1(alpm_pkg_get_version(pkg)) == version()) && (QString::fromLatin1((db_name = (char *)alpm_db_get_name(alpm_pkg_get_db(pkg)))?db_name:"local") == repo())) break;
            pkg = NULL;
        } while(dbs.next());
        dbs.detach();
        return pkg;
    }

    return NULL;
}

QVector<AlpmPackage::UserChangeStatus> AlpmPackage::possibleChangeStatuses() const {
    if (isInstalled() && (repo() == "local" || repo() == "aur")) return QVector<AlpmPackage::UserChangeStatus>() << DO_UNINSTALL_ALL << DO_UNINSTALL;
    else if (isInstalled()) return QVector<AlpmPackage::UserChangeStatus>() << DO_REINSTALL << DO_REINSTALL_ASDEPS << DO_UNINSTALL_ALL << DO_UNINSTALL;
    return QVector<AlpmPackage::UserChangeStatus>() << DO_INSTALL << DO_INSTALL_ASDEPS << DO_INSTALL_FORCE << DO_INSTALL_ASDEPS_FORCE;
}

bool AlpmPackage::setChangeStatus(const UserChangeStatus & status) {
    if (!possibleChangeStatuses().contains(status)) return false;
    m_change_status = status;
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
        for (Dependence dep: provides()) {
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

