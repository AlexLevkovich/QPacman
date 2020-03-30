/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "alpmdb.h"
#include "libalpm.h"
#include <QThread>
#include <QDebug>

template<class ForwardIt, class T, class Compare> static ForwardIt binary_search_ex(ForwardIt first, ForwardIt last, const T& value, Compare comp) {
    ForwardIt it = std::lower_bound(first, last, value, comp);
    if ((it != last) && comp(value,*it)) it = last;
    return it;
}

QMap <QString,QVector<AlpmPackage *> > AlpmDB::m_packages;
QMap <QString,QStringList> AlpmDB::m_groups;
QMap <QString,QMap<AlpmPackage::Dependence,QVector<qint64> > > AlpmDB::m_provides;

void AlpmDB::check_error() const {
    if (Alpm::p_alpm != NULL && Alpm::p_alpm->m_alpm_handle == NULL) Alpm::p_alpm->m_alpm_errno = Alpm::ALPM_HANDLE_FAILED;
    if (m_db_handle == NULL) Alpm::p_alpm->m_alpm_errno = Alpm::ALPMDB_HANDLE_FAILED;
}

template<class T> T & AlpmDB::check_error(T & t) const {
    check_error();
    return t;
}

template<class T> T AlpmDB::check_error(const T & t) const {
    check_error();
    return t;
}

AlpmDB::AlpmDB() {
    m_db_handle = NULL;
}

AlpmDB::AlpmDB(alpm_db_t * db_handle) {
    m_db_handle = db_handle;
}

AlpmDB::AlpmDB(const AlpmDB & db) {
    m_db_handle = db.m_db_handle;
}

AlpmDB::~AlpmDB() {}

void AlpmDB::clean_pkg_cache(const QString & dbname) {
    if (dbname.isEmpty()) return;

    QVector<AlpmPackage *> & cache = m_packages[dbname];
    while (!cache.isEmpty()) delete cache.takeFirst();
    m_groups[dbname].clear();
    m_provides[dbname].clear();
}

void AlpmDB::clean_pkg_caches() {
    QMapIterator<QString,QVector<AlpmPackage *> > i(m_packages);
    while (i.hasNext()) {
        i.next();
        clean_pkg_cache(i.key());
    }
    m_packages.clear();
    m_groups.clear();
    m_provides.clear();
}

const QVector<AlpmPackage *> & AlpmDB::packages() const {
    QString dbname = name();
    if (dbname.isEmpty()) return m_packages[QString()];

    if (!m_packages[dbname].isEmpty()) return m_packages[dbname];

    alpm_list_t * ret = alpm_db_get_pkgcache(m_db_handle);
    if (ret == NULL) {
        qCritical() << Alpm::instance()->lastError();
        return m_packages[QString()];
    }

    clean_pkg_cache(dbname);
    AlpmPackage * pkg;
    AlpmList<alpm_pkg_t> pkgs(ret,AlpmList<alpm_pkg_t>::ignorefree);
    do {
        if (pkgs.isEmpty()) break;
        pkg = new AlpmPackage(pkgs.valuePtr());
        m_groups[dbname] += pkg->groups();
        for (qint64 i=0;i<pkg->provides().count();i++) {
            (m_provides[dbname])[pkg->provides()[i]].append(m_packages[dbname].count());
        }
        m_packages[dbname].append(pkg);
    } while (pkgs.next());
    pkgs.detach();

    m_groups[dbname].removeDuplicates();
    return m_packages[dbname];
}

const QStringList & AlpmDB::groups() const {
    QString dbname = name();
    if (dbname.isEmpty()) return check_error(m_groups[QString()]);

    if (!m_groups[dbname].isEmpty()) return m_groups[dbname];

    packages();

    return m_groups[dbname];
}

bool AlpmDB::isValid() const {
    return (m_db_handle != NULL && Alpm::p_alpm != NULL && Alpm::p_alpm->m_alpm_handle != NULL && !alpm_db_get_valid(m_db_handle));
}

QString AlpmDB::name() const {
    if (!isValid()) return check_error(QString());

    const char * m_name = alpm_db_get_name(m_db_handle);
    if (m_name == NULL) return QString();

    return QString::fromLocal8Bit(m_name);
}

const QString AlpmDB::extension() {
    return AlpmConfig::dbExtension();
}

bool AlpmDB::update(bool force) {
    if (!isValid()) return check_error(false);

    if (name() == QString::fromLatin1("local")) {
        Alpm::p_alpm->m_alpm_errno=Alpm::LOCAL_DB_UPDATE;
        return false;
    }

    Alpm::p_alpm->m_download_errs.clear();
    return (alpm_db_update(force?1:0,m_db_handle) >= 0);
}

QVector<AlpmPackage *> AlpmAbstractDB::find(const QRegularExpression & expr) const {
    QVector<AlpmPackage *> ret;

    QList<AlpmPackage::Dependence> provides;
    QRegularExpression exp(expr);
    exp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    const QVector<AlpmPackage *> & pkgs = packages();
    for (qint64 i=0;i<pkgs.count();i++) {
        if (exp.match(pkgs[i]->name()).hasMatch() || exp.match(pkgs[i]->description()).hasMatch()) ret.append(pkgs[i]);
        else {
            provides = pkgs[i]->provides();
            for (qint64 j=0;j<provides.count();j++) {
                if (exp.match(provides[j].toString()).hasMatch()) {
                    ret.append(pkgs[i]);
                    break;
                }
            }
        }
    }

    return ret;
}

QVector<qint64> AlpmAbstractDB::findCacheIndexes(const QRegularExpression & expr) const {
    QVector<qint64> ret;

    QList<AlpmPackage::Dependence> provides;
    QRegularExpression exp(expr);
    exp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    const QVector<AlpmPackage *> & pkgs = packages();
    for (qint64 i=0;i<pkgs.count();i++) {
        if (exp.match(pkgs[i]->name()).hasMatch() || exp.match(pkgs[i]->description()).hasMatch()) ret.append(i);
        else {
            provides = pkgs[i]->provides();
            for (int j=0;j<provides.count();j++) {
                if (exp.match(provides[j].toString()).hasMatch()) {
                    ret.append(i);
                    break;
                }
            }
        }
    }

    return ret;
}

QVector<AlpmPackage *> AlpmAbstractDB::findByGroup(const QString & group) const {
    QVector<AlpmPackage *> ret;
    const QVector<AlpmPackage *> & pkgs = packages();
    for (qint64 i=0;i<pkgs.count();i++) {
        if (pkgs[i]->groups().contains(group)) ret.append(pkgs[i]);
    }

    return ret;
}

QVector<AlpmPackage *> AlpmAbstractDB::findByGroup(const char * group) const {
    return findByGroup(QString::fromLatin1(group));
}

QVector<qint64> AlpmAbstractDB::findCacheIndexesByGroup(const QString & group) const {
    QVector<qint64> ret;
    const QVector<AlpmPackage *> & pkgs = packages();
    for (qint64 i=0;i<pkgs.count();i++) {
        if (pkgs[i]->groups().contains(group)) ret.append(i);
    }

    return ret;
}

AlpmDB & AlpmDB::operator=(const AlpmDB & other) {
    m_db_handle = other.m_db_handle;
    return *this;
}

QVector<AlpmPackage *> AlpmAbstractDB::findByFileName(const QString & filename) const {
    return findByFileName(filename.toLocal8Bit().constData());
}

QVector<AlpmPackage *> AlpmAbstractDB::findByFileName(const char * filename) const {
    QVector<AlpmPackage *> ret;
    const QVector<AlpmPackage *> & pkgs = packages();
    for (qint64 i=0;i<pkgs.count();i++) {
        alpm_pkg_t * pkg = pkgs[i]->handle();
        if (pkg == NULL) continue;
        alpm_filelist_t * files = alpm_pkg_get_files(pkg);
        if (files == NULL) continue;
        if(alpm_filelist_contains(files,filename)) {
            ret.append(pkgs[i]);
        }
    }

    return ret;
}

QVector<qint64> AlpmAbstractDB::findCacheIndexesByFileName(const QString & _filename) const {
    QVector<qint64> ret;
    const char * filename = _filename.toLocal8Bit().constData();
    const QVector<AlpmPackage *> & pkgs = packages();
    for (qint64 i=0;i<pkgs.count();i++) {
        alpm_pkg_t * pkg = pkgs[i]->handle();
        if (pkg == NULL) continue;
        alpm_filelist_t * files = alpm_pkg_get_files(pkg);
        if (files == NULL) continue;
        if(alpm_filelist_contains(files,filename)) {
            ret.append(i);
        }
    }

    return ret;
}

QVector<AlpmPackage *> AlpmAbstractDB::findByPackageNameProvides(const AlpmPackage::Dependence & provide) const {
    QVector<AlpmPackage *> ret;
    QVector<qint64> indexes = findCacheIndexesByPackageNameProvides(provide);
    if (indexes.count() <= 0) return ret;

    for (qint64 i=0;i<indexes.count();i++) {
        ret.append(packages()[indexes[i]]);
    }

    return ret;
}

const QMap<AlpmPackage::Dependence,QVector<qint64> > & AlpmDB::provides() const {
    return m_provides[name()];
}

QVector<qint64> AlpmAbstractDB::findCacheIndexesByPackageNameProvides(const AlpmPackage::Dependence & provide) const {
    packages();

    return provides().value(provide,QVector<qint64>());
}

bool AlpmAbstractDB::no_version_asc_less(AlpmPackage * item1, AlpmPackage * item2) {
    return (item1->name() < item2->name());
}

bool AlpmAbstractDB::asc_version_equal(AlpmPackage * item1, AlpmPackage * item2) {
    return !AlpmPackage::pkg_vercmp(item1->version(),item2->version());
}

bool AlpmAbstractDB::no_version_asc_equal(AlpmPackage * item1, AlpmPackage * item2) {
    return (item1->name() == item2->name());
}

QVector<AlpmPackage *> AlpmAbstractDB::findByPackageName(const QString & pkgname) const {
    QVector<AlpmPackage *> ret;
    const QVector<AlpmPackage *> & pkgs = packages();
    AlpmPackage entry(pkgname,QString());
    QVector<AlpmPackage *>::const_iterator it = binary_search_ex(pkgs.begin(),pkgs.end(),&entry,no_version_asc_less);
    if (it == pkgs.end()) return ret;
    ret.append(pkgs[it-pkgs.begin()]);

    qint64 i;
    for (i=(it-pkgs.begin()-1);i>=0;i--) {
        if (no_version_asc_equal(pkgs[i],pkgs[it-pkgs.begin()])) ret.append(pkgs[i]);
        else break;
    }
    for (i=((it-pkgs.begin())+1);i<pkgs.count();i++) {
        if (no_version_asc_equal(pkgs[i],pkgs[it-pkgs.begin()])) ret.append(pkgs[i]);
        else break;
    }


    return ret;
}

QVector<AlpmPackage *> AlpmAbstractDB::findByPackageName(const char * pkgname) const {
    return findByPackageName(QString::fromLatin1(pkgname));
}

QVector<qint64> AlpmAbstractDB::findCacheIndexesByPackageName(const QString & pkgname) const {
    QVector<qint64> ret;
    const QVector<AlpmPackage *> & pkgs = packages();
    AlpmPackage entry(pkgname,QString());
    QVector<AlpmPackage *>::const_iterator it = binary_search_ex(pkgs.begin(),pkgs.end(),&entry,no_version_asc_less);
    if (it == pkgs.end()) return ret;
    ret.append(it-pkgs.begin());

    qint64 i;
    for (i=(it-pkgs.begin()-1);i>=0;i--) {
        if (no_version_asc_equal(pkgs[i],pkgs[it-pkgs.begin()])) ret.append(i);
        else break;
    }
    for (i=((it-pkgs.begin())+1);i<pkgs.count();i++) {
        if (no_version_asc_equal(pkgs[i],pkgs[it-pkgs.begin()])) ret.append(i);
        else break;
    }


    return ret;
}

QVector<AlpmPackage *> AlpmAbstractDB::findByPackageNameVersion(const QString & pkgname,const QString & version) const {
    QVector<AlpmPackage *> ret;
    QVector<qint64> indexes = findCacheIndexesByPackageName(pkgname);
    if (indexes.count() <= 0) return ret;

    AlpmPackage pkg(pkgname,version);
    const QVector<AlpmPackage *> & pkgs = packages();
    for (int i=0;i<indexes.count();i++) {
        if (asc_version_equal(pkgs[indexes[i]],&pkg)) ret.append(pkgs[indexes[i]]);
    }

    return ret;
}

QVector<AlpmPackage *> AlpmAbstractDB::findByPackageNameVersion(const char * pkgname,const char * version) const {
    return findByPackageNameVersion(QString::fromLatin1(pkgname),QString::fromLatin1(version));
}

QVector<qint64> AlpmAbstractDB::findCacheIndexesByPackageNameVersion(const QString & pkgname,const QString & version) const {
    QVector<qint64> ret;
    QVector<qint64> indexes = findCacheIndexesByPackageName(pkgname);
    if (indexes.count() <= 0) return ret;

    AlpmPackage pkg(pkgname,version);
    const QVector<AlpmPackage *> & pkgs = packages();
    for (int i=0;i<indexes.count();i++) {
        if (asc_version_equal(pkgs[indexes[i]],&pkg)) ret.append(indexes[i]);
    }

    return ret;
}

AlpmPackage * AlpmAbstractDB::findByPackageNameVersionRepo(const QString & pkgname,const QString & version,const QString & repo) const {
    QVector<AlpmPackage *> ret = findByPackageNameVersion(pkgname,version);
    if (ret.count() <= 0) return NULL;

    for (int i=0;i<ret.count();i++) {
        if (ret[i]->repo() == repo) return ret[i];
    }

    return NULL;
}

AlpmPackage * AlpmAbstractDB::findByPackageNameVersionRepo(const char * pkgname,const char * version,const char * repo) const {
    return AlpmAbstractDB::findByPackageNameVersionRepo(QString::fromLatin1(pkgname),QString::fromLatin1(version),QString::fromLocal8Bit(repo));
}

qint64 AlpmAbstractDB::findCacheIndexByPackageNameVersionRepo(const QString & pkgname,const QString & version,const QString & repo) const {
    QVector<qint64> ret = findCacheIndexesByPackageNameVersion(pkgname,version);
    if (ret.count() <= 0) return -1;

    for (int i=0;i<ret.count();i++) {
        if (packages()[ret[i]]->repo() == repo) return ret[i];
    }

    return -1;
}
