/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef PACMANPACKAGE_H
#define PACMANPACKAGE_H

#include <alpm.h>
#include <QStringList>
#include <QUrl>
#include <QMap>
#include <QDateTime>
#include <QRegularExpression>
#include "alpmlist.h"
#include <AppStreamQt/component.h>
#include <QDataStream>
#include <QFileInfo>

class Alpm;
class AlpmDB;
namespace AppStream {
class Pool;
}

class AlpmPackage {
public:
    typedef alpm_list_t * (*alpm_pkg_deplist_fn)(alpm_pkg_t * pkg);

    enum Type {
        Local = 0,
        File = 1,
        SyncDB = 2,
        Unknown = -1
    };

    enum PackageFilter {
        IS_ALL = 0,
        IS_INSTALLED = 1,
        IS_NONINSTALLED = 2,
        IS_NEEDUPDATE = 3,
        IS_ORPHANED = 4,
        IS_GROUP = 5,
        IS_MARKED = 6
    };

    enum Reason {
        Explicit = 0,
        Depend = 1,
        Undefined = 3
    };

    enum CompareOper {
        UNKNOWN = 0,
        EQUAL = 1,
        MORE_OR_EQUAL = 2,
        LESS_OR_EQUAL = 3,
        MORE = 4,
        LESS = 5
    };

    enum UserChangeStatus {
        DO_INSTALL = 1,
        DO_INSTALL_ASDEPS = 2,
        DO_INSTALL_FORCE = 4,
        DO_INSTALL_ASDEPS_FORCE = 8,
        DO_UNINSTALL_ALL = 16,
        DO_UNINSTALL = 32,
        DO_REINSTALL = 64,
        DO_REINSTALL_ASDEPS = 128,
        DO_NOTHING = 256
    };

    enum SearchFieldType {
        DESC = 0,
        NAME = 1,
        FILE_NAME = 2,
        PROVIDER = 3,
        DEPENDENCY = 4
    };

    class FileInfo {
    private:
        QString m_path;
        qint64 m_size;
        mode_t m_mode;
        QDateTime m_date;

        FileInfo(const alpm_file_t & path);
        FileInfo(const QString & path,qint64 size,mode_t mode,const QDateTime & date);
        mode_t toMode_t(const QFileInfo & info);
    public:
        FileInfo();
        QString path() const;
        qint64 size() const;
        mode_t mode() const;
        QDateTime date() const;

        friend QDataStream & operator<<(QDataStream &argument,const AlpmPackage::FileInfo & path);
        friend const QDataStream & operator>>(const QDataStream &argument,AlpmPackage::FileInfo & path);

        friend class AlpmPackage;
    };

    class Dependence {
    private:
        QString m_name;
        QString m_version;
        QString m_description;
        CompareOper m_operation;

        static CompareOper mod_to_compareoper(alpm_depmod_t mod);
        void init(const QString & name,CompareOper operation = UNKNOWN,const QString & version = QString(),const QString & description = QString());
        void init(const AlpmPackage & pkg,CompareOper operation = EQUAL);

    public:
        Dependence();
        Dependence(const QString & name,CompareOper operation = UNKNOWN,const QString & version = QString(),const QString & description = QString());
        Dependence(const AlpmPackage & pkg,CompareOper operation = EQUAL);
        Dependence(const Dependence & dep);

        QString name() const;
        QString version() const;
        QString description() const;
        CompareOper operation() const;
        QString toString() const;
        bool isAppropriate(const AlpmPackage & pkg) const;
        bool isAppropriate(const QString & name,const QString & version) const;
        bool isAppropriate(const Dependence & dep) const;
        bool operator==(const Dependence & dep);
        bool operator<(const Dependence & dep);
        Dependence & operator=(const Dependence &dep);
        QList<Dependence> findDepends(uint provider_id = 0) const;
        bool isInstalled() const;
        alpm_depend_t to_alpm_depend() const;
        static const Dependence fromString(const QString & str);
        friend QDataStream & operator<<(QDataStream &argument,const AlpmPackage::Dependence & dep);
        friend const QDataStream & operator>>(const QDataStream &argument,AlpmPackage::Dependence & dep);
        friend class AlpmPackage;
        friend class Alpm;
    };

    AlpmPackage();
    AlpmPackage(const AlpmPackage & pkg);
    AlpmPackage(const AlpmPackage * pkg);
    AlpmPackage(const QString & filename,bool do_delete = true);
    AlpmPackage(const QString & name,const QString & version, const QString & dbname);
    ~AlpmPackage();
    bool isValid() const;

    QLatin1String        name() const;
    QLatin1String        version() const;
    QLatin1String        description() const;
    QUrl                 url() const;
    QDateTime            buildDate() const;
    QDateTime            installDate() const;
    QString              packager() const;
    QLatin1String        arch() const;
    QStringList          licenses() const;
    QStringList          groups() const;
    QList<Dependence>    depends() const;
    QList<Dependence>    optdepends() const;
    QList<Dependence>    conflicts() const;
    QList<Dependence>    provides() const;
    QList<Dependence>    replaces() const;
    QStringList          requiredby() const;
    QStringList          optionalfor() const;
    QList<FileInfo>      files() const;
    Type                 type() const;
    QLatin1String        repo() const;
    qint64               size() const;
    qint64               installedSize() const;
    Reason               reason() const;
    bool                 setReason(Reason reason);
    QString              download();

    friend QDataStream & operator<<(QDataStream &argument,const AlpmPackage & pkg);
    friend const QDataStream & operator>>(const QDataStream &argument,AlpmPackage & pkg);
    static int registerMetaType();

    bool isUpdate() const;
    // checks that it might be an update: there are no restrictions in config
    // returns true if it needs to be ignored
    bool isIgnorable() const;

    AlpmPackage & operator=(const AlpmPackage &other);
    bool operator==(const AlpmPackage &other);

    UserChangeStatus changeStatus() const;
    static const QList<UserChangeStatus> possibleChangeStatuses(const AlpmPackage & pkg);
    static void resetAllChangeStatuses();
    static bool setChangeStatus(const AlpmPackage & pkg,UserChangeStatus status);
    UserChangeStatus defaultStatus() const;
    QList<UserChangeStatus> possibleChangeStatuses() const;

    bool hasFile(const QString & path) const;
    bool hasFileAs(const QRegularExpression & expr) const;
    bool hasProviderAs(const QRegularExpression & expr) const;
    bool hasDescriptionAs(const QRegularExpression & expr) const;
    bool hasNameAs(const QRegularExpression & expr) const;
    bool isOrphaned() const;
    bool isInstalled() const;
    bool isDownloaded(QString * path_pkg_file = NULL) const;
    bool isFile() const;
    //inName - search in name() otherwise - in description()
    bool containsText(const QString & text,SearchFieldType field);
    bool ownedByGroup(const QString & group);

    QString toString() const;
    QString fileName() const;
    QList<QLatin1String> remoteLocations() const;
    QString filePath() const;

    QUrl iconUrl() const;

    // by default local pkg delete flag is true and pkg is deleted at destruction
    // it is avoided by sync pkgs
    void setLocalDeleteFlag(bool flag) { m_delete = flag; }
    bool localDeleteFlag() { return m_delete; }

    static const QList<AlpmPackage> changedStatusPackages();
    static CompareOper parseNameVersion(const QString & str,QString & name,QString & ver);
    static CompareOper parseNameVersion(const QString & str,QString & repo,QString & name,QString & ver);
    static int pkg_vercmp(const QString & ver1, const QString & ver2);

protected:
    AlpmPackage(alpm_pkg_t *pkg);
    static const AlpmPackage newPackage(const QString & name,const QString & version);

private:
    QList<FileInfo> files(const QString & archive_path) const;
    QList<FileInfo> files(alpm_pkg_t * pkg) const;
    Reason intToReason(alpm_pkgreason_t reason) const;
    alpm_pkgreason_t reasonToInt(Reason reason) const;
    static int pkg_version_compare(alpm_pkg_t * item1, alpm_pkg_t * item2);
    static const QList<Dependence> alpm_pkg_list_processing(alpm_pkg_deplist_fn alpm_pkg_deplist,alpm_pkg_t *m_pkg);
    static bool splitname_ver(const QString & target,QString & name,QString & version);
    alpm_pkg_t * handle() const;
    alpm_pkg_t * findInLocal(alpm_pkg_t * pkg) const;
    alpm_pkg_t * findInSync(alpm_pkg_t * pkg) const;
    QUrl icon_url(const QString & name) const;

    alpm_pkg_t * m_handle;
    QString m_filepath;
    bool m_delete;

    class alpm_pkg_t_p {
    public:
        alpm_pkg_t_p(alpm_pkg_t * ptr) {
            m_ptr = ptr;
        }
        alpm_pkg_t_p() {
            m_ptr = NULL;
        }
        alpm_pkg_t * ptr() const {
            return m_ptr;
        }
    private:
        alpm_pkg_t * m_ptr;
    };

    static QMap<alpm_pkg_t_p,UserChangeStatus> m_change_statuses;
    friend bool operator<(const alpm_pkg_t_p & e1, const alpm_pkg_t_p & e2);

    static AppStream::Pool * m_pool;
    static QMultiHash<QString, AppStream::Component> m_appInfo;

    friend class Alpm;
    friend class AlpmDB;
    friend class HandleReleaser;
};
Q_DECLARE_METATYPE(AlpmPackage);
Q_DECLARE_METATYPE(AlpmPackage::Dependence);
Q_DECLARE_METATYPE(AlpmPackage::FileInfo);

bool operator<(const AlpmPackage::Dependence & dep1, const AlpmPackage::Dependence & dep2);

#endif // PACMANPACKAGE_H
