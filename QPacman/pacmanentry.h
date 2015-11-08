/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANENTRY_H
#define PACMANENTRY_H

#include <QDateTime>
#include <QDataStream>
#include <QStringList>
#ifndef IS_QPACMAN_SERVER
#include "categorytoolbutton.h"
#endif

class PacmanItemModel;
class PacmanSimpleItemModel;

class PacmanEntry {
    friend class PacmanItemModel;
    friend class PacmanSimpleItemModel;
public:
    PacmanEntry();
    PacmanEntry(const QString & name,const QString & ver);
    PacmanEntry(const QString & name);

    enum InstalledStatus {
        INSTALLED_EXPLICITLY = 0,
        INSTALLED_AS_DEP = 1,
        NOT_INSTALLED = 2
    };

    enum UserChangeStatus {
        DO_INSTALL = 0,
        DO_UNINSTALL = 1,
        DO_REINSTALL = 2
    };

    struct Deps {
        QString package;
        QString desc;

        friend QDataStream & operator<<(QDataStream & stream,const Deps & entry);
        friend QDataStream & operator>>(QDataStream & stream,Deps & entry);
    };

    enum ParseCode {
        OK = 0,
        EMPTY = 1,
        WARNINGS = 2
    };

    enum StringListFlags {
        unknown_flag = -1,
        optionaldeps_flag = 0
    };

    enum CompareOper {
        MORE = 0,
        MORE_OR_EQUAL = 1,
        LESS = 2,
        LESS_OR_EQUAL = 3,
        EQUAL = 4,
        UNKNOWN = 5
    };

#ifdef IS_QPACMAN_CLIENT
    static const QString fieldNames[19];
#endif
    bool isValid() const { return !name.isNull(); }
    ParseCode parseLine(const QByteArray & array);
    QString getName() const {return name;}
    InstalledStatus getStatus() const {return status;}
    UserChangeStatus getChangeStatus() const {return change_status;}
    void setChangeStatus(const UserChangeStatus & status) {
        if (((status != DO_INSTALL) && (getStatus() != NOT_INSTALLED)) ||
            ((status == DO_INSTALL) && (getStatus() == NOT_INSTALLED))) {
            change_status = status;
        }
    }
    QString getRepo() const {return repo;}
    QString getVersion() const {return version;}
    QString getDescription() const {return desc;}
    QString getArch() const {return arch;}
    QString getURL() const {return url;}
    QString getLicense() const {return lic;}
    QStringList listGroups() const {return groups;}
    QStringList listProvides() const {return provides;}
    QStringList listDependOn() const {return dependon;}
    QList<Deps> listOptionalDeps() const {return optionaldeps;}
    QStringList listRequiredBy() const {return requiredby;}
    QStringList listOptionalFor() const {return optionalfor;}
    QStringList listConflictsTo() const {return conflicts;}
    QStringList listReplaces() const {return replaces;}
    QStringList listWarnings() const {return warnings;}
    QStringList listFiles() const {return files;}
    void setFiles(const QStringList & files) { this->files = files; }
    qreal getInstallationSize() const {return instsize;}
    QString getPackager() const {return packager;}
    QDateTime buildDate() const {return builddate;}
    QDateTime installDate() const {return installdate;}
    bool isInstalled() const { return (status != NOT_INSTALLED); }
    bool isUpdate() const { return m_isUpdate; }
    bool isChosen() const { return m_isChosen; }
    bool isLocal() const { return repo == "aur"; }
    bool isExplicitly() const { return (status == INSTALLED_EXPLICITLY); }
    void revertStatus() { status = (status == INSTALLED_EXPLICITLY)?INSTALLED_AS_DEP:INSTALLED_EXPLICITLY; }
    bool isOrphaned();
    bool ownedByGroup(const QString & group);
#ifdef IS_QPACMAN_CLIENT
    QString toHtml() const;
#endif

#ifndef IS_QPACMAN_SERVER
    bool containsText(const QString & text,CategoryToolButton::ItemId cItemId);
#endif
    QString toString() const;
    static const QDateTime parseDate(const QString & str);
    static const QString afterColon(const QString & str);
    static const QString afterEqualSign(const QString & str);
    static const QString removeVersion(const QString & str);
    static CompareOper parseNameVersion(const QString & str,QString & name,QString & ver);
    static const QString parsePackageFileNameVersion(const QString & package_file);
    static const QStringList entriesListToStringList(const QList<PacmanEntry> & list);
    static const QStringList entriesListToNamesStringList(const QList<PacmanEntry> & list);
    static const QString compareOperToString(CompareOper oper);

    friend QDataStream & operator<<(QDataStream & stream,const PacmanEntry & entry);
    friend QDataStream & operator>>(QDataStream & stream,PacmanEntry & entry);

private:
    QString name;
    QString repo;
    QString version;
    QString desc;
    QString arch;
    QString url;
    QString lic;
    QStringList groups;
    QStringList provides;
    QStringList dependon;
    QList<Deps> optionaldeps;
    QStringList requiredby;
    QStringList optionalfor;
    QStringList conflicts;
    QStringList replaces;
    qreal instsize;
    QString packager;
    QDateTime builddate;
    QDateTime installdate;
    QStringList warnings;
    QStringList files;
    bool m_isUpdate;
    bool m_isChosen;
    InstalledStatus status;
    UserChangeStatus change_status;
    StringListFlags _flag;

    static bool isMailAddress(const QString & str);
    static const QString modifyMailAddress(const QString & str);
    static const QStringList parseStrList(const QString & str);
    static const QStringList parseStrList2(const QString & str);
    static void afterBeforeColon(const QString & line,QString & str_before,QString & str_after);
    static bool splitname_ver(const QString & target,QString & name,QString & version);
    static const QString removeInstalledWord(const QString & str,bool * result = NULL);
};

inline bool operator==(const PacmanEntry &e1, const PacmanEntry &e2) {
    return e1.getName() == e2.getName() && e1.getVersion() == e2.getVersion();
}

inline uint qHash(const PacmanEntry &key) {
    return qHash(key.getName()+key.getVersion());
}

#endif // PACMANENTRY_H
