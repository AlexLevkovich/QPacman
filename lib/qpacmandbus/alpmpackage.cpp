/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "alpmpackage.h"
#include "libalpm.h"

const int metareg = AlpmPackage::registerMetaType();

static void parseEVR(char *evr, const char **ep, const char **vp,
        const char **rp)
{
    const char *epoch;
    const char *version;
    const char *release;
    char *s, *se;

    s = evr;
    /* s points to epoch terminator */
    while (*s && isdigit(*s)) s++;
    /* se points to version terminator */
    se = strrchr(s, '-');

    if(*s == ':') {
        epoch = evr;
        *s++ = '\0';
        version = s;
        if(*epoch == '\0') {
            epoch = "0";
        }
    } else {
        /* different from RPM- always assume 0 epoch */
        epoch = "0";
        version = evr;
    }
    if(se) {
        *se++ = '\0';
        release = se;
    } else {
        release = NULL;
    }

    if(ep) *ep = epoch;
    if(vp) *vp = version;
    if(rp) *rp = release;
}

/**
 * Compare alpha and numeric segments of two versions.
 * return 1: a is newer than b
 *        0: a and b are the same version
 *       -1: b is newer than a
 */
static int rpmvercmp(const char *a, const char *b)
{
    char oldch1, oldch2;
    char *str1, *str2;
    char *ptr1, *ptr2;
    char *one, *two;
    int rc;
    int isnum;
    int ret = 0;

    /* easy comparison to see if versions are identical */
    if(strcmp(a, b) == 0) return 0;

    str1 = strdup(a);
    str2 = strdup(b);

    one = ptr1 = str1;
    two = ptr2 = str2;

    /* loop through each version segment of str1 and str2 and compare them */
    while (*one && *two) {
        while (*one && !isalnum((int)*one)) one++;
        while (*two && !isalnum((int)*two)) two++;

        /* If we ran to the end of either, we are finished with the loop */
        if (!(*one && *two)) break;

        /* If the separator lengths were different, we are also finished */
        if ((one - ptr1) != (two - ptr2)) {
            ret = (one - ptr1) < (two - ptr2) ? -1 : 1;
            goto cleanup;
        }

        ptr1 = one;
        ptr2 = two;

        /* grab first completely alpha or completely numeric segment */
        /* leave one and two pointing to the start of the alpha or numeric */
        /* segment and walk ptr1 and ptr2 to end of segment */
        if (isdigit((int)*ptr1)) {
            while (*ptr1 && isdigit((int)*ptr1)) ptr1++;
            while (*ptr2 && isdigit((int)*ptr2)) ptr2++;
            isnum = 1;
        } else {
            while (*ptr1 && isalpha((int)*ptr1)) ptr1++;
            while (*ptr2 && isalpha((int)*ptr2)) ptr2++;
            isnum = 0;
        }

        /* save character at the end of the alpha or numeric segment */
        /* so that they can be restored after the comparison */
        oldch1 = *ptr1;
        *ptr1 = '\0';
        oldch2 = *ptr2;
        *ptr2 = '\0';

        /* this cannot happen, as we previously tested to make sure that */
        /* the first string has a non-null segment */
        if (one == ptr1) {
            ret = -1;	/* arbitrary */
            goto cleanup;
        }

        /* take care of the case where the two version segments are */
        /* different types: one numeric, the other alpha (i.e. empty) */
        /* numeric segments are always newer than alpha segments */
        /* XXX See patch #60884 (and details) from bugzilla #50977. */
        if (two == ptr2) {
            ret = isnum ? 1 : -1;
            goto cleanup;
        }

        if (isnum) {
            /* this used to be done by converting the digit segments */
            /* to ints using atoi() - it's changed because long  */
            /* digit segments can overflow an int - this should fix that. */

            /* throw away any leading zeros - it's a number, right? */
            while (*one == '0') one++;
            while (*two == '0') two++;

            /* whichever number has more digits wins */
            if (strlen(one) > strlen(two)) {
                ret = 1;
                goto cleanup;
            }
            if (strlen(two) > strlen(one)) {
                ret = -1;
                goto cleanup;
            }
        }

        /* strcmp will return which one is greater - even if the two */
        /* segments are alpha or if they are numeric.  don't return  */
        /* if they are equal because there might be more segments to */
        /* compare */
        rc = strcmp(one, two);
        if (rc) {
            ret = rc < 1 ? -1 : 1;
            goto cleanup;
        }

        /* restore character that was replaced by null above */
        *ptr1 = oldch1;
        one = ptr1;
        *ptr2 = oldch2;
        two = ptr2;
    }

    /* this catches the case where all numeric and alpha segments have */
    /* compared identically but the segment separating characters were */
    /* different */
    if ((!*one) && (!*two)) {
        ret = 0;
        goto cleanup;
    }

    /* the final showdown. we never want a remaining alpha string to
     * beat an empty string. the logic is a bit weird, but:
     * - if one is empty and two is not an alpha, two is newer.
     * - if one is an alpha, two is newer.
     * - otherwise one is newer.
     * */
    if ( (!*one && !isalpha((int)*two))
            || isalpha((int)*one) ) {
        ret = -1;
    } else {
        ret = 1;
    }

cleanup:
    free(str1);
    free(str2);
    return ret;
}

/** Compare two version strings and determine which one is 'newer'.
 * Returns a value comparable to the way strcmp works. Returns 1
 * if a is newer than b, 0 if a and b are the same version, or -1
 * if b is newer than a.
 *
 * Different epoch values for version strings will override any further
 * comparison. If no epoch is provided, 0 is assumed.
 *
 * Keep in mind that the pkgrel is only compared if it is available
 * on both versions handed to this function. For example, comparing
 * 1.5-1 and 1.5 will yield 0; comparing 1.5-1 and 1.5-2 will yield
 * -1 as expected. This is mainly for supporting versioned dependencies
 * that do not include the pkgrel.
 */
int alpm_pkg_vercmp(const char *a, const char *b)
{
    char *full1, *full2;
    const char *epoch1, *ver1, *rel1;
    const char *epoch2, *ver2, *rel2;
    int ret;

    /* ensure our strings are not null */
    if(!a && !b) {
        return 0;
    } else if(!a) {
        return -1;
    } else if(!b) {
        return 1;
    }
    /* another quick shortcut- if full version specs are equal */
    if(strcmp(a, b) == 0) {
        return 0;
    }

    /* Parse both versions into [epoch:]version[-release] triplets. We probably
     * don't need epoch and release to support all the same magic, but it is
     * easier to just run it all through the same code. */
    full1 = strdup(a);
    full2 = strdup(b);

    /* parseEVR modifies passed in version, so have to dupe it first */
    parseEVR(full1, &epoch1, &ver1, &rel1);
    parseEVR(full2, &epoch2, &ver2, &rel2);

    ret = rpmvercmp(epoch1, epoch2);
    if(ret == 0) {
        ret = rpmvercmp(ver1, ver2);
        if(ret == 0 && rel1 && rel2) {
            ret = rpmvercmp(rel1, rel2);
        }
    }

    free(full1);
    free(full2);
    return ret;
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

const QDataStream & operator>>(const QDataStream &argument,AlpmPackage & pkg) {
    (QDataStream &)argument >> pkg.m_handle;
    (QDataStream &)argument >> pkg.m_name;
    (QDataStream &)argument >> pkg.m_version;
    (QDataStream &)argument >> pkg.m_desc;
    (QDataStream &)argument >> pkg.m_dbname;
    (QDataStream &)argument >> pkg.m_isUpdate;
    (QDataStream &)argument >> pkg.m_installed;
    (QDataStream &)argument >> pkg.m_filepath;
    return argument;
}

QDataStream & operator<<(QDataStream &argument,const AlpmPackage & pkg) {
    argument << pkg.m_handle;
    argument << pkg.m_filepath;
    argument << (bool)false;

    return argument;
}

AlpmPackage::Dependence::Dependence() {
    m_operation = UNKNOWN;
    m_installed = false;
}

AlpmPackage::Dependence::Dependence(const QString & name,CompareOper operation,const QString & version,const QString & description) {
    m_name = name;
    m_version = version;
    m_description = description;
    m_operation = operation;
    if (m_operation == UNKNOWN) m_version.clear();
}

int AlpmPackage::registerMetaType() {
    qRegisterMetaType<AlpmPackage>("AlpmPackage");
    qRegisterMetaType<QList<AlpmPackage> >("QList<AlpmPackage>");
    qRegisterMetaType<AlpmPackage::Dependence>("AlpmPackage::Dependence");
    qRegisterMetaType<AlpmPackage::FileInfo>("AlpmPackage::FileInfo");
    return 0;
}

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

bool AlpmPackage::Dependence::operator<(const Dependence & dep) {
    return (compare_deps(*this,dep) < 0);
}

bool AlpmPackage::Dependence::operator==(const Dependence & dep) {
    return !compare_deps(*this,dep);
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

bool AlpmPackage::Dependence::isInstalled() const {
    AlpmPackage pkg = Alpm::instance()->findLocalPackage(name());
    if (pkg.isValid()) return isAppropriate(pkg.name(),pkg.version());

    for (AlpmPackage & pkg: Alpm::instance()->findLocalByPackageNameProvides(*this)) {
        if (pkg.isInstalled()) return true;
    }
    return false;
}

QString AlpmPackage::Dependence::urlParms() const {
    if (operation() == AlpmPackage::UNKNOWN) return description().isEmpty()?name():(name()+"?descr="+description());
    return name()+QString("?oper=%1&ver=%2%3").arg(operation()).arg(version()).arg(description().isEmpty()?"":"&descr="+description());
}

QUrl AlpmPackage::Dependence::url(const QString & type) const {
    return QString("qpc://%1/%2").arg(type).arg(urlParms());
}

int AlpmPackage::Dependence::pkg_vercmp(const QString & ver1, const QString & ver2) {
    return alpm_pkg_vercmp(ver1.toLatin1().constData(),ver2.toLatin1().constData());
}

bool AlpmPackage::Dependence::isAppropriate(const QString & name,const QString & version) const {
    if (m_name != name) return false;
    if (m_operation == UNKNOWN) return true;
    int ret = pkg_vercmp(version,m_version);
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

AlpmPackage::AlpmPackage() {
    m_handle = 0;
    m_isUpdate = false;
    m_installed = false;
    m_isOrphaned = 'U';
    m_type = AlpmPackage::Unknown;
    m_pkg_size = -1;
    m_pkg_isize = -1;
    delete_at_destruction = false;
}

AlpmPackage::AlpmPackage(const QString & name) {
    m_handle = 0;
    m_name = name.toLatin1();
    m_isUpdate = false;
    m_installed = false;
    m_isOrphaned = 'U';
    m_type = AlpmPackage::Unknown;
    m_pkg_size = -1;
    m_pkg_isize = -1;
    delete_at_destruction = false;
}

AlpmPackage::AlpmPackage(const QString & name,const QString & version) {
    m_handle = 0;
    m_name = name.toLatin1();
    m_version = version.toLatin1();
    m_isUpdate = false;
    m_installed = false;
    m_isOrphaned = 'U';
    m_type = AlpmPackage::Unknown;
    m_pkg_size = -1;
    m_pkg_isize = -1;
    delete_at_destruction = false;
}

AlpmPackage::AlpmPackage(const QString & pkgpath,bool delete_at_destruction) {
    m_handle = 0;
    m_type = AlpmPackage::Unknown;
    m_isUpdate = false;
    m_installed = false;
    m_isOrphaned = 'U';
    m_pkg_size = -1;
    m_pkg_isize = -1;
    delete_at_destruction = false;
    if (Alpm::instance() == NULL) return;
    *this = Alpm::instance()->createLocalPackage(pkgpath);
    this->delete_at_destruction = delete_at_destruction;
}

AlpmPackage::~AlpmPackage() {
    if (Alpm::instance() != NULL && m_type == AlpmPackage::File && delete_at_destruction) {
        Alpm::instance()->deleteLocalPackage(*this);
    }
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

bool AlpmPackage::isValid() const {
    return ((type() != AlpmPackage::Type::Unknown) && !m_name.isEmpty() && !m_desc.isEmpty());
}

QLatin1String AlpmPackage::name() const {
    return QLatin1String(m_name);
}

QLatin1String AlpmPackage::version() const {
    return QLatin1String(m_version);
}

QLatin1String AlpmPackage::description() const {
    return QLatin1String(m_desc);
}

QUrl AlpmPackage::url() const {
    if (Alpm::instance() == NULL) return QUrl();
    if (m_url.isValid()) return m_url;

    return (((AlpmPackage *)this)->m_url = Alpm::instance()->packageUrl(*this));
}

QString AlpmPackage::fileName() const {
    if (Alpm::instance() == NULL) return QString();
    if (!m_filename.isEmpty()) return m_filename;

    return (((AlpmPackage *)this)->m_filename = Alpm::instance()->packageFileName(*this));
}

QString AlpmPackage::filePath() const {
    return m_filepath;
}

QDateTime AlpmPackage::buildDate() const {
    if (Alpm::instance() == NULL) return QDateTime();
    if (m_build_date.isValid()) return m_build_date;

    return (((AlpmPackage *)this)->m_build_date = Alpm::instance()->packageBuildDate(*this));
}

QDateTime AlpmPackage::installDate() const {
    if (Alpm::instance() == NULL) return QDateTime();
    if (m_install_date.isValid()) return m_install_date;

    return (((AlpmPackage *)this)->m_install_date = Alpm::instance()->packageInstallDate(*this));
}

QString AlpmPackage::packager() const {
    if (Alpm::instance() == NULL) return QString();
    if (!m_packager.isEmpty()) return m_packager;

    return (((AlpmPackage *)this)->m_packager = Alpm::instance()->packagePackager(*this));
}

QLatin1String AlpmPackage::arch() const {
    if (Alpm::instance() == NULL) return QLatin1String();
    if (!m_arch.isEmpty()) return QLatin1String(m_arch);

    return QLatin1String(((AlpmPackage *)this)->m_arch = Alpm::instance()->packageArch(*this));
}

QLatin1String AlpmPackage::repo() const {
    return QLatin1String(m_dbname);
}

bool AlpmPackage::isInstalled() const {
    return m_installed;
}

AlpmPackage::UserChangeStatus AlpmPackage::changeStatus() const {
    if (Alpm::instance() == NULL) return AlpmPackage::DO_NOTHING;

    return Alpm::instance()->packageChangeStatus(*this);
}

QStringList AlpmPackage::licenses() const {
    if (Alpm::instance() == NULL) return QStringList();
    if (!m_licenses.isEmpty()) return m_licenses;

    return ((AlpmPackage *)this)->m_licenses = Alpm::instance()->packageLicenses(*this);
}

QStringList AlpmPackage::requiredby() const {
    if (Alpm::instance() == NULL) return QStringList();
    if (!m_requiredby.isEmpty()) return m_requiredby;

    return ((AlpmPackage *)this)->m_requiredby = Alpm::instance()->packageRequiredby(*this);
}

QStringList AlpmPackage::optionalfor() const {
    if (Alpm::instance() == NULL) return QStringList();
    if (!m_optionalfor.isEmpty()) return m_optionalfor;

    return ((AlpmPackage *)this)->m_optionalfor = Alpm::instance()->packageOptionalfor(*this);
}

QStringList AlpmPackage::groups() const {
    if (Alpm::instance() == NULL) return QStringList();
    if (!m_groups.isEmpty()) return m_groups;

    return ((AlpmPackage *)this)->m_groups = Alpm::instance()->packageGroups(*this);
}

QList<AlpmPackage::Dependence> AlpmPackage::depends() const {
    if (Alpm::instance() == NULL) return QList<AlpmPackage::Dependence>();
    if (!m_depends.isEmpty()) return m_depends;

    return ((AlpmPackage *)this)->m_depends = Alpm::instance()->packageDepends(*this);
}

QList<AlpmPackage::Dependence> AlpmPackage::optdepends() const {
    if (Alpm::instance() == NULL) return QList<AlpmPackage::Dependence>();
    if (!m_optdepends.isEmpty()) return m_optdepends;

    return ((AlpmPackage *)this)->m_optdepends = Alpm::instance()->packageOptDepends(*this);
}

QList<AlpmPackage::Dependence> AlpmPackage::conflicts() const {
    if (Alpm::instance() == NULL) return QList<AlpmPackage::Dependence>();
    if (!m_conflicts.isEmpty()) return m_conflicts;

    return ((AlpmPackage *)this)->m_conflicts = Alpm::instance()->packageConflicts(*this);
}

QList<AlpmPackage::Dependence> AlpmPackage::provides() const {
    if (Alpm::instance() == NULL) return QList<AlpmPackage::Dependence>();
    if (!m_provides.isEmpty()) return m_provides;

    return ((AlpmPackage *)this)->m_provides = Alpm::instance()->packageProvides(*this);
}

QList<AlpmPackage::Dependence> AlpmPackage::replaces() const {
    if (Alpm::instance() == NULL) return QList<AlpmPackage::Dependence>();
    if (!m_replaces.isEmpty()) return m_replaces;

    return ((AlpmPackage *)this)->m_replaces = Alpm::instance()->packageReplaces(*this);
}

AlpmPackage::Type AlpmPackage::type() const {
    if (Alpm::instance() == NULL) return AlpmPackage::Unknown;
    if (m_type != AlpmPackage::Unknown) return m_type;

    return ((AlpmPackage *)this)->m_type = Alpm::instance()->packageType(*this);
}

qint64 AlpmPackage::size() const {
    if (Alpm::instance() == NULL) return -1;
    if (m_pkg_size >= 0) return m_pkg_size;

    return ((AlpmPackage *)this)->m_pkg_size = Alpm::instance()->packageSize(*this);
}

qint64 AlpmPackage::installedSize() const {
    if (Alpm::instance() == NULL) return -1;
    if (m_pkg_isize >= 0) return m_pkg_isize;

    return ((AlpmPackage *)this)->m_pkg_isize = Alpm::instance()->packageInstalledSize(*this);
}

AlpmPackage::Reason AlpmPackage::reason() const {
    if (Alpm::instance() == NULL) return AlpmPackage::Undefined;

    return Alpm::instance()->packageReason(*this);
}

bool AlpmPackage::setReason(AlpmPackage::Reason reason) {
    if (Alpm::instance() == NULL) return false;

    return Alpm::instance()->packageSetReason(*this,reason);
}

bool AlpmPackage::isOrphaned() const {
    if (Alpm::instance() == NULL) return false;

    if (m_isOrphaned == 'U') ((AlpmPackage *)this)->m_isOrphaned = Alpm::instance()->packageIsOrphaned(*this)?'Y':'N';
    return (m_isOrphaned == 'Y');
}

bool AlpmPackage::setChangeStatus(AlpmPackage::UserChangeStatus status) {
    if (Alpm::instance() == NULL) return false;

    return Alpm::instance()->packageSetChangeStatus(*this,status);
}

QList<AlpmPackage::UserChangeStatus> AlpmPackage::possibleChangeStatuses() const {
    if (Alpm::instance() == NULL) return QList<AlpmPackage::UserChangeStatus>();

    return Alpm::instance()->packagePossibleChangeStatuses(*this);
}

AlpmPackage::UserChangeStatus AlpmPackage::defaultStatus() const {
    if (Alpm::instance() == NULL) return AlpmPackage::DO_NOTHING;

    return Alpm::instance()->packageDefaultStatus(*this);
}

QUrl AlpmPackage::iconUrl() const {
    if (Alpm::instance() == NULL) return QUrl();
    if (m_icon_url.isValid()) return m_icon_url;

    return (((AlpmPackage *)this)->m_icon_url = Alpm::instance()->packageIconUrl(*this));
}

QList<AlpmPackage::FileInfo> AlpmPackage::files() const {
    if (Alpm::instance() == NULL) return QList<AlpmPackage::FileInfo>();

    return Alpm::instance()->packageFiles(*this);
}

bool AlpmPackage::isDownloaded() const {
    if (Alpm::instance() == NULL) return false;

    return Alpm::instance()->packageIsDownloaded(*this);
}

bool AlpmPackage::isUpdate() const {
    return m_isUpdate;
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

AlpmPackage::FileInfo::FileInfo() {
    m_mode = 0;
    m_size = 0;
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

void AlpmPackage::FileInfo::setPath(const QString & path) {
    m_path = path;
}

void AlpmPackage::resetAllChangeStatuses() {
    if (Alpm::instance() == NULL) return;

    Alpm::instance()->resetPackageChangeStatuses();
}
