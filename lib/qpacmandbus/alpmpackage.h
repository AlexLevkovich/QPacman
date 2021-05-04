/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef ALPMPACKAGE_H
#define ALPMPACKAGE_H

#include <QStringList>
#include <QDateTime>
#include <QUrl>
#include <QByteArray>
#include <QDataStream>

class AlpmPackage {
public:

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

    public:
        FileInfo();
        QString path() const;
        qint64 size() const;
        mode_t mode() const;
        QDateTime date() const;
        void setPath(const QString & path);

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
        bool m_installed;

        QString urlParms() const;

    public:
        Dependence();
        Dependence(const QString & name,CompareOper operation = UNKNOWN,const QString & version = QString(),const QString & description = QString());

        QString name() const;
        QString version() const;
        QString description() const;
        CompareOper operation() const;
        QString toString() const;
        bool operator<(const Dependence &dep);
        bool operator==(const Dependence &dep);
        bool isAppropriate(const QString & name,const QString & version) const;

        bool isInstalled() const;
        QUrl url(const QString & type) const;
        static int pkg_vercmp(const QString & ver1, const QString & ver2);

        friend const QDataStream & operator>>(const QDataStream &argument,AlpmPackage::Dependence & dep);
        friend QDataStream & operator<<(QDataStream &argument,const AlpmPackage::Dependence & dep);
    };

    AlpmPackage();
    AlpmPackage(const QString & name);
    AlpmPackage(const QString & name,const QString & version);
    AlpmPackage(const QString & pkgpath,bool delete_at_destruction);
    ~AlpmPackage();
    bool isValid() const;

    static int registerMetaType();

    bool isLocalPackageDeletionAllowed() { return delete_at_destruction; }
    void setLocalPackageDeletionAllowed(bool flag) { delete_at_destruction = flag; }

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

    bool isUpdate() const;
    // checks that it might be an update: there are no restrictions in config
    // returns true if it needs to be ignored
    bool isIgnorable() const;
    bool isOrphaned() const;
    bool isInstalled() const;

    UserChangeStatus changeStatus() const;
    UserChangeStatus defaultStatus() const;
    QList<UserChangeStatus> possibleChangeStatuses() const;
    bool setChangeStatus(UserChangeStatus status);

    QString fileName() const;
    QString filePath() const;
    QUrl iconUrl() const;
    bool isDownloaded() const;

    static CompareOper parseNameVersion(const QString & str,QString & name,QString & ver);
    static CompareOper parseNameVersion(const QString & str,QString & repo,QString & name,QString & ver);
    static void resetAllChangeStatuses();

private:
    qulonglong m_handle;
    qulonglong m_alpm_handle;
    QByteArray m_name;
    QByteArray m_version;
    QByteArray m_desc;
    QStringList m_groups;
    QList<Dependence> m_provides;
    QByteArray m_dbname;
    bool m_installed;
    bool m_isUpdate;
    char m_isOrphaned;
    QString m_filepath;
    QUrl m_url;
    QUrl m_icon_url;
    QString m_filename;
    QByteArray m_arch;
    QStringList m_remotelocs;
    QDateTime m_build_date;
    QDateTime m_install_date;
    QString m_packager;
    QStringList m_licenses;
    QStringList m_requiredby;
    QStringList m_optionalfor;
    QList<Dependence> m_depends;
    QList<Dependence> m_optdepends;
    QList<Dependence> m_conflicts;
    QList<Dependence> m_replaces;
    AlpmPackage::Type m_type;
    qint64 m_pkg_size;
    qint64 m_pkg_isize;
    bool delete_at_destruction;

    static bool splitname_ver(const QString & target,QString & name,QString & version);
    friend const QDataStream & operator>>(const QDataStream &argument,AlpmPackage & pkg);
    friend QDataStream & operator<<(QDataStream &argument,const AlpmPackage & pkg);
};
Q_DECLARE_METATYPE(AlpmPackage);
Q_DECLARE_METATYPE(AlpmPackage::Dependence);
Q_DECLARE_METATYPE(AlpmPackage::FileInfo);

#endif // ALPMPACKAGE_H
