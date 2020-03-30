/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef PACMANPACKAGE_H
#define PACMANPACKAGE_H

#include <alpm.h>
#include <QStringList>
#include <QUrl>
#include <QHash>
#include <QDateTime>
#include <QRegularExpression>
#include "alpmlist.h"
#include <AppStreamQt/component.h>

class Alpm;
class AlpmDB;
namespace AppStream {
class Pool;
}

typedef int (*calculate_first_column_width)(const QString * field_names,int count);

class AlpmPackage {
public:
    typedef alpm_list_t * (*alpm_pkg_deplist_fn)(alpm_pkg_t * pkg);

    enum Type {
        Local = ALPM_PKG_FROM_LOCALDB,
        File = ALPM_PKG_FROM_FILE,
        SyncDB = ALPM_PKG_FROM_SYNCDB,
        Unknown = -1
    };

    enum Reason {
        Explicit = ALPM_PKG_REASON_EXPLICIT,
        Depend = ALPM_PKG_REASON_DEPEND,
        NotInstalled,
        Undefined
    };

    enum CompareOper {
        UNKNOWN = ALPM_DEP_MOD_ANY,
        EQUAL = ALPM_DEP_MOD_EQ,
        MORE_OR_EQUAL = ALPM_DEP_MOD_GE,
        LESS_OR_EQUAL = ALPM_DEP_MOD_LE,
        MORE = ALPM_DEP_MOD_GT,
        LESS = ALPM_DEP_MOD_LT
    };

    enum UserChangeStatus {
        DO_INSTALL = 1,
        DO_INSTALL_ASDEPS = 2,
        DO_INSTALL_FORCE = 4,
        DO_INSTALL_ASDEPS_FORCE = 8,
        DO_UNINSTALL_ALL = 16,
        DO_UNINSTALL = 32,
        DO_REINSTALL = 64,
        DO_REINSTALL_ASDEPS = 128
    };

    class Dependence {
    private:
        QString m_name;
        QString m_version;
        QString m_description;
        CompareOper m_operation;

        QString urlParms() const;

    public:
        Dependence();
        Dependence(const QString & name,CompareOper operation = UNKNOWN,const QString & version = QString(),const QString & description = QString());
        Dependence(AlpmPackage * pkg,CompareOper operation = EQUAL);

        void init(const QString & name,CompareOper operation = UNKNOWN,const QString & version = QString(),const QString & description = QString());
        void init(AlpmPackage * pkg,CompareOper operation = EQUAL);
        QString name() const;
        QString version() const;
        QString description() const;
        CompareOper operation() const;
        QString toString() const;
        bool isAppropriate(const AlpmPackage * pkg) const;
        bool isAppropriate(const QString & name,const QString & version) const;
        bool isAppropriate(const Dependence & dep) const;
        bool operator==(const Dependence & dep);
        bool operator<(const Dependence & dep);
        QList<Dependence> findDepends(uint provider_id = 0) const;
        bool isInstalled() const;
        QUrl url(const QString & type) const;
        alpm_depend_t to_alpm_depend() const;
    };

    AlpmPackage();
    AlpmPackage(const AlpmPackage & pkg);
    AlpmPackage(const AlpmPackage * pkg);
    AlpmPackage(const QString & filename);
    AlpmPackage(const QString & name,const QString & version);
    bool isValid() const;

    QString           name() const;
    QString           version() const;
    QString           description() const;
    QUrl              url() const;
    QDateTime         buildDate() const;
    QDateTime         installDate() const;
    QString           packager() const;
    QString           arch() const;
    QStringList       licenses() const;
    QStringList       groups() const;
    QList<Dependence> depends() const;
    QList<Dependence> optdepends() const;
    QList<Dependence> conflicts() const;
    QList<Dependence> provides() const;
    QList<Dependence> replaces() const;
    QStringList       requiredby() const;
    QStringList       optionalfor() const;
    QStringList       files() const;
    Type              type() const;
    QString           repo() const;
    off_t             size() const;
    off_t             installedSize() const;
    Reason            reason() const;
    bool              setReason(Reason reason);
    QString           download();

    bool isUpdate() const { return m_isUpdate; }
    // checks that it might be an update: there are no restrictions in config
    // returns true if it needs to be ignored
    bool isIgnorable() const { return m_isIgnorable; }

    AlpmPackage & operator=(const AlpmPackage &other);
    bool operator==(const AlpmPackage &other);

    UserChangeStatus changeStatus() const {return m_change_status; }
    bool setChangeStatus(const UserChangeStatus & status);
    QVector<UserChangeStatus> possibleChangeStatuses() const;

    bool hasFileAs(const QRegularExpression & expr) const;
    bool hasProviderAs(const QRegularExpression & expr) const;
    bool hasDescriptionAs(const QRegularExpression & expr) const;
    bool hasNameAs(const QRegularExpression & expr) const;
    bool isOrphaned() const;
    bool isInstalled() const;
    bool isChosen() const { return m_isChosen; }
    bool isDownloaded(QString * path_pkg_file = NULL) const;
    bool isFile() const;
    //inName - search in name() otherwise - in description()
    bool containsText(const QString & text,bool inName);
    bool ownedByGroup(const QString & group);

    QString toString() const;
    QString fileName() const;
    QStringList remoteLocations() const;
    QString filePath() const;

    QUrl iconUrl() const;

    static CompareOper parseNameVersion(const QString & str,QString & name,QString & ver);
    static CompareOper parseNameVersion(const QString & str,QString & repo,QString & name,QString & ver);
    static int pkg_vercmp(const QString & ver1, const QString & ver2);

protected:
    AlpmPackage(alpm_pkg_t *pkg);

private:
    static const QList<Dependence> alpm_pkg_list_processing(alpm_pkg_deplist_fn alpm_pkg_deplist,alpm_pkg_t *m_pkg);
    static bool splitname_ver(const QString & target,QString & name,QString & version);
    void init(alpm_pkg_t * m_pkg);
    alpm_pkg_t * handle() const;
    void setInstalled(bool installed);

    QString m_name;
    QString m_version;
    QString m_desc;
    QStringList m_groups;
    QList<Dependence> m_provides;
    QString m_dbname;
    bool m_installed;
    UserChangeStatus m_change_status;
    bool m_isChosen;
    bool m_isUpdate;
    bool m_isIgnorable;
    Reason m_reason;
    QString m_filepath;
    QUrl m_url;
    QString m_filename;
    QString m_arch;
    QStringList m_remotelocs;
    QDateTime m_build_date;
    QDateTime m_install_date;
    QString m_packager;
    QStringList m_licenses;
    QList<Dependence> m_depends;
    QList<Dependence> m_optdepends;
    QList<Dependence> m_conflicts;
    QList<Dependence> m_replaces;
    QStringList m_requiredby;
    QStringList m_optionalfor;
    AlpmPackage::Type m_type;
    off_t m_pkg_size;
    off_t m_pkg_isize;

    static AppStream::Pool * m_pool;
    static QHash<QString, AppStream::Component> m_appInfo;

    friend class Alpm;
    friend class AlpmDB;
    friend class AlpmAbstractDB;
    friend class PacmanItemModel;
    friend class PacmanSimpleItemModel;
};
Q_DECLARE_METATYPE(AlpmPackage);

bool operator<(const AlpmPackage::Dependence & dep1, const AlpmPackage::Dependence & dep2);

#endif // PACMANPACKAGE_H
