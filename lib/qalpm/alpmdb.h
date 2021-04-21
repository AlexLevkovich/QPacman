/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ALPMDB_H
#define ALPMDB_H

#include <QList>
#include <QMap>
#include <QList>
#include <QLatin1String>
#include <QRegularExpression>
#include "alpmpackage.h"

typedef struct __alpm_db_t alpm_db_t;

class AlpmDB {
public:
    AlpmDB(const AlpmDB & db);
    AlpmDB(const QString & name);
    virtual ~AlpmDB();
    bool isValid() const;
    QLatin1String name() const;
    bool update(bool force = false);

    AlpmDB & operator=(const AlpmDB &other);
    static const QString extension();

    QList<AlpmPackage> packages(const QString & str = QString(),AlpmPackage::SearchFieldType fieldType = AlpmPackage::NAME,AlpmPackage::PackageFilter filter = AlpmPackage::IS_ALL,const QString & group = QString(),const QString & dbname = QString()) const;
    const QStringList & groups();

    QList<AlpmPackage> find(const QRegularExpression & expr) const;
    AlpmPackage findByFileName(const QString & filename) const;
    AlpmPackage findByFileName(const char * filename) const;
    QList<AlpmPackage> findByGroup(const char * group) const;
    QList<AlpmPackage> findByGroup(const QString & group) const;
    AlpmPackage findByPackageName(const QString & pkgname) const;
    AlpmPackage findByPackageName(const char * pkgname) const;
    AlpmPackage findByPackageNameVersion(const QString & pkgname,const QString & version) const;
    AlpmPackage findByPackageNameVersion(const char * pkgname,const char * version) const;
    QList<AlpmPackage> findByPackageNameProvides(const AlpmPackage::Dependence & provide) const;

protected:
    AlpmDB();
    AlpmDB(alpm_db_t * db_handle);
    const QMap<AlpmPackage::Dependence,QList<AlpmPackage> > & provides();

private:
    static int no_version_compare(alpm_pkg_t * item1, alpm_pkg_t * item2);
    template<class T> T & check_error(T & t,const char * err = NULL) const;
    template<class T> T check_error(const T & t,const char * err = NULL) const;
    void check_error(const char * err = NULL) const;
    bool isAppropriateDepsForPackageName(const QString & name,const QList<AlpmPackage::Dependence> & deps) const;

    alpm_db_t * m_db_handle;
    QStringList m_groups;
    QMap<AlpmPackage::Dependence,QList<AlpmPackage> > m_provides;

    friend class Alpm;
    friend class AlpmPackage;
};

#endif // ALPMDB_H
