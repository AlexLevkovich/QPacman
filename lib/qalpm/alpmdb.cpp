/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "alpmdb.h"
#include <dlfcn.h>
#include <QThread>
#include "libalpm.h"
#include "alpmlist.h"
#include <alpm.h>
#include <QDebug>

template<class ForwardIt, class T, class Compare> static ForwardIt binary_search_ex(ForwardIt first, ForwardIt last, const T& value, Compare comp) {
    ForwardIt it = std::lower_bound(first, last, value, comp);
    if ((it == last) || comp(value,*it)) it = last;
    return it;
}

template<class ForwardIt, class T> static ForwardIt binary_search_ex(ForwardIt first, ForwardIt last, const T& value) {
    ForwardIt it = std::lower_bound(first, last, value);
    if ((it == last) || (value < *it)) it = last;
    return it;
}

void AlpmDB::check_error(const char * err) const {
    if (!isValid()) {
        Alpm::instance()->m_alpm_errno = ALPM_ERR_HANDLE_NULL;
        if (err != nullptr) qCritical() << err;
    }
}

template<class T> T & AlpmDB::check_error(T & t,const char * err) const {
    check_error(err);
    return t;
}

template<class T> T AlpmDB::check_error(const T & t,const char * err) const {
    check_error(err);
    return t;
}

AlpmDB::AlpmDB() {
    m_db_handle = nullptr;
    m_alpm_handle = (Alpm::instance() == nullptr)?nullptr:Alpm::instance()->m_alpm_handle;
}

AlpmDB::AlpmDB(alpm_db_t * db_handle) {
    m_db_handle = db_handle;
    m_alpm_handle = (Alpm::instance() == nullptr)?nullptr:Alpm::instance()->m_alpm_handle;
}

AlpmDB::AlpmDB(const QString & name) {
    m_db_handle = nullptr;
    m_alpm_handle = (Alpm::instance() == nullptr)?nullptr:Alpm::instance()->m_alpm_handle;
    if (!isValid()) {
        Alpm::instance()->m_alpm_errno = ALPM_ERR_HANDLE_NULL;
        return;
    }
    if (name == "local") {
        m_db_handle = alpm_get_localdb(Alpm::instance()->m_alpm_handle);
    }
    else {
        AlpmList<alpm_db_t> dbs(alpm_get_syncdbs(Alpm::instance()->m_alpm_handle),AlpmList<alpm_db_t>::ignorefree);
        do {
            if (name == alpm_db_get_name(dbs.valuePtr())) {
                m_db_handle = dbs.valuePtr();
                break;
            }
        } while(dbs.goNext());
        dbs.detach();
    }
}

AlpmDB::AlpmDB(const AlpmDB & db) {
    m_db_handle = db.m_db_handle;
    m_alpm_handle = db.m_alpm_handle;
}

bool AlpmDB::isAppropriateDepsForPackageName(const QString & name,const QList<AlpmPackage::Dependence> & deps) const {
    for (const AlpmPackage::Dependence & dep: deps) {
        if (dep.isAppropriate(AlpmPackage::Dependence::fromString(name))) return true;
    }

    return false;
}

QList<AlpmPackage> AlpmDB::packages(const QString & str,AlpmPackage::SearchFieldType fieldType,AlpmPackage::PackageFilter filter,const QString & group) const {
    QList<AlpmPackage> m_packages;

    if (!isValid()) return check_error(m_packages);

    Alpm::instance()->m_alpm_errno = ALPM_ERR_OK;

    AlpmPackage pkg;
    AlpmList<alpm_pkg_t> pkgs(alpm_db_get_pkgcache(m_db_handle),AlpmList<alpm_pkg_t>::ignorefree);
    do {
        if (pkgs.isEmpty()) break;
        pkg = AlpmPackage(pkgs.valuePtr());

        if (filter != AlpmPackage::IS_ALL) {
            if (filter == AlpmPackage::IS_ORPHANED) { if (!pkg.isOrphaned()) continue; }
            else if (filter == AlpmPackage::IS_INSTALLED) { if (!pkg.isInstalled()) continue; }
            else if (filter == AlpmPackage::IS_NONINSTALLED) { if (pkg.isInstalled()) continue; }
            else if (filter == AlpmPackage::IS_NEEDUPDATE) { if (!pkg.isUpdate()) continue; }
            else if (filter == AlpmPackage::IS_MARKED) { if (pkg.changeStatus() == AlpmPackage::DO_NOTHING) continue; }
            else if (filter == AlpmPackage::IS_GROUP) { if (!group.isEmpty() && !pkg.groups().contains(group)) continue; }
        }
        if (!str.isEmpty()) {
            if (fieldType == AlpmPackage::NAME) { if (!pkg.name().contains(str,Qt::CaseInsensitive)) continue; }
            else if (fieldType == AlpmPackage::DESC) { if (!pkg.description().contains(str,Qt::CaseInsensitive)) continue; }
            else if (fieldType == AlpmPackage::PROVIDER) { if (!isAppropriateDepsForPackageName(str.toLower(),pkg.provides())) continue; }
            else if (fieldType == AlpmPackage::DEPENDENCY) { if (!isAppropriateDepsForPackageName(str.toLower(),pkg.depends())) continue; }
            else if (fieldType == AlpmPackage::FILE_NAME) { if (!pkg.hasFile(str)) continue; }
        }

        m_packages.append(pkg);
    } while (pkgs.goNext());
    pkgs.detach();

    return m_packages;
}

const QMap<AlpmPackage::Dependence,QList<AlpmPackage> > & AlpmDB::provides() {
    if (!m_provides.isEmpty()) return m_provides;

    if (!isValid()) return check_error(m_provides,"Wrong db handle!!!");
    Alpm::instance()->m_alpm_errno = ALPM_ERR_OK;

    AlpmPackage pkg;
    AlpmList<alpm_pkg_t> pkgs(alpm_db_get_pkgcache(m_db_handle),AlpmList<alpm_pkg_t>::ignorefree);
    do {
        if (pkgs.isEmpty()) break;
        pkg = AlpmPackage(pkgs.valuePtr());
        for (AlpmPackage::Dependence & dep: pkg.provides()) {
            ((AlpmDB *)this)->m_provides[dep].append(pkg);
        }
    } while (pkgs.goNext());
    pkgs.detach();

    return m_provides;
}

QStringList AlpmDB::groups() const {
    if (!m_groups.isEmpty()) return m_groups;

    if (!isValid()) return check_error(m_groups,"Wrong db handle!!!");
    Alpm::instance()->m_alpm_errno = ALPM_ERR_OK;

    AlpmDB * p_this = (AlpmDB *)this;
    AlpmList<alpm_pkg_t> pkgs(alpm_db_get_pkgcache(m_db_handle),AlpmList<alpm_pkg_t>::ignorefree);
    do {
        if (pkgs.isEmpty()) break;
        p_this->m_groups += AlpmPackage(pkgs.valuePtr()).groups();
    } while (pkgs.goNext());
    pkgs.detach();

    p_this->m_groups.removeDuplicates();

    return m_groups;
}

bool AlpmDB::isValid() const {
    return (m_db_handle != nullptr && Alpm::instance() != nullptr && Alpm::instance()->m_alpm_handle != nullptr && (Alpm::instance()->m_alpm_handle == m_alpm_handle));
}

QLatin1String AlpmDB::name() const {
    if (!isValid()) return QLatin1String();

    const char * name = alpm_db_get_name(m_db_handle);
    return QLatin1String((name == nullptr)?"":name);
}

const QString AlpmDB::extension() {
    return AlpmConfig::dbExtension();
}

bool AlpmDB::update(bool force) {
    if (!isValid()) return check_error(false,"Wrong db handle!!!");
    Alpm::instance()->m_alpm_errno = ALPM_ERR_OK;

    Alpm::p_alpm->m_download_errs.clear();
    m_groups.clear();
    m_provides.clear();

    AlpmList<alpm_db_t> pkgs(AlpmList<alpm_db_t>::ignorefree);
    pkgs.append(m_db_handle);
    return (alpm_db_update(Alpm::instance()->m_alpm_handle,pkgs.alpm_list(),force?1:0) >= 0);
}

QList<AlpmPackage> AlpmDB::find(const QRegularExpression & expr) const {
    QList<AlpmPackage> ret;

    if (!isValid()) return check_error(ret,"Wrong db handle!!!");
    Alpm::instance()->m_alpm_errno = ALPM_ERR_OK;

    AlpmPackage pkg;
    QList<AlpmPackage::Dependence> provides;
    QRegularExpression exp(expr);
    exp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    AlpmList<alpm_pkg_t> pkgs(alpm_db_get_pkgcache(m_db_handle),AlpmList<alpm_pkg_t>::ignorefree);
    do {
        if (pkgs.isEmpty()) break;
        pkg = AlpmPackage(pkgs.valuePtr());
        if (exp.match(pkg.name()).hasMatch() || exp.match(pkg.description()).hasMatch()) ret.append(pkg);
        else {
            provides = pkg.provides();
            for (qint64 j=0;j<provides.count();j++) {
                if (exp.match(provides[j].toString()).hasMatch()) {
                    ret.append(pkg);
                    break;
                }
            }
        }

    } while (pkgs.goNext());
    pkgs.detach();

    return ret;
}

QList<AlpmPackage> AlpmDB::findByGroup(const QString & group) const {
    QList<AlpmPackage> ret;

    if (!isValid()) return check_error(ret,"Wrong db handle!!!");
    Alpm::instance()->m_alpm_errno = ALPM_ERR_OK;

    AlpmPackage pkg;
    AlpmList<alpm_pkg_t> pkgs(alpm_db_get_pkgcache(m_db_handle),AlpmList<alpm_pkg_t>::ignorefree);
    do {
        if (pkgs.isEmpty()) break;
        pkg = AlpmPackage(pkgs.valuePtr());
        if (pkg.groups().contains(group)) ret.append(pkg);
    } while (pkgs.goNext());
    pkgs.detach();

    return ret;
}

QList<AlpmPackage> AlpmDB::findByGroup(const char * group) const {
    return findByGroup(QString::fromLatin1(group));
}

AlpmDB & AlpmDB::operator=(const AlpmDB & other) {
    m_groups.clear();
    m_provides.clear();
    m_db_handle = other.m_db_handle;
    m_alpm_handle = other.m_alpm_handle;
    return *this;
}

AlpmPackage AlpmDB::findByFileName(const QString & filename) const {
    return findByFileName(filename.toLocal8Bit().constData());
}

AlpmPackage AlpmDB::findByFileName(const char * filename) const {
    AlpmPackage ret;

    if (!isValid()) return check_error(ret,"Wrong db handle!!!");
    Alpm::instance()->m_alpm_errno = ALPM_ERR_OK;

    alpm_pkg_t * pkg;
    AlpmList<alpm_pkg_t> pkgs(alpm_db_get_pkgcache(m_db_handle),AlpmList<alpm_pkg_t>::ignorefree);
    do {
        if (pkgs.isEmpty()) break;
        pkg = pkgs.valuePtr();
        if (pkg == nullptr) continue;
        alpm_filelist_t * files = alpm_pkg_get_files(pkg);
        if (files == nullptr) continue;
        if(alpm_filelist_contains(files,filename)) {
            ret = AlpmPackage(pkg);
            break;
        }
    } while (pkgs.goNext());
    pkgs.detach();

    return ret;
}

QList<AlpmPackage> AlpmDB::findByPackageNameProvides(const AlpmPackage::Dependence & provide) const {
    QList<AlpmPackage> ret;

    QList<AlpmPackage::Dependence> keys = ((AlpmDB *)this)->provides().keys();
    QList<AlpmPackage::Dependence>::const_iterator it = binary_search_ex(keys.begin(),keys.end(),provide);
    if (it == keys.end()) return ret;

    for (;(it != keys.constEnd()) && (*it).isAppropriate(provide);it++) {
        ret.append(m_provides[*it]);
    }

    return ret;
}

int AlpmDB::no_version_compare(alpm_pkg_t * item1, alpm_pkg_t * item2) {
    return strcmp(alpm_pkg_get_name(item1),alpm_pkg_get_name(item2));
}

AlpmPackage AlpmDB::findByPackageName(const QString & pkgname) const {
    return AlpmPackage(alpm_db_get_pkg(m_db_handle,pkgname.toLatin1().constData()));
}

AlpmPackage AlpmDB::findByPackageName(const char * pkgname) const {
    return findByPackageName(QString::fromLatin1(pkgname));
}

AlpmPackage AlpmDB::findByPackageNameVersion(const QString & pkgname,const QString & version) const {
    AlpmPackage ret = findByPackageName(pkgname);
    if (!ret.isValid()) return ret;
    if (ret.version() == version) return ret;
    return AlpmPackage();
}

AlpmPackage AlpmDB::findByPackageNameVersion(const char * pkgname,const char * version) const {
    return findByPackageNameVersion(QString::fromLatin1(pkgname),QString::fromLatin1(version));
}
