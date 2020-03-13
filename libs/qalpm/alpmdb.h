/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ALPMDB_H
#define ALPMDB_H

#include <QList>
#include <QMap>
#include <QVector>
#include <QRegularExpression>
#include "alpmpackage.h"

class AlpmAbstractDB {
public:
    QVector<AlpmPackage *> find(const QRegularExpression & expr) const;
    QVector<qint64> findCacheIndexes(const QRegularExpression & expr) const;
    QVector<AlpmPackage *> findByFileName(const QString & filename) const;
    QVector<AlpmPackage *> findByFileName(const char * filename) const;
    QVector<qint64> findCacheIndexesByFileName(const QString & filename) const;
    QVector<AlpmPackage *> findByGroup(const char * group) const;
    QVector<AlpmPackage *> findByGroup(const QString & group) const;
    QVector<qint64> findCacheIndexesByGroup(const QString & group) const;
    QVector<AlpmPackage *> findByPackageName(const QString & pkgname) const;
    QVector<AlpmPackage *> findByPackageName(const char * pkgname) const;
    QVector<qint64> findCacheIndexesByPackageName(const QString & pkgname) const;
    QVector<AlpmPackage *> findByPackageNameVersion(const QString & pkgname,const QString & version) const;
    QVector<AlpmPackage *> findByPackageNameVersion(const char * pkgname,const char * version) const;
    QVector<qint64> findCacheIndexesByPackageNameVersion(const QString & pkgname,const QString & version) const;
    AlpmPackage * findByPackageNameVersionRepo(const QString & pkgname,const QString & version,const QString & repo) const;
    AlpmPackage * findByPackageNameVersionRepo(const char * pkgname,const char * version,const char * repo) const;
    qint64 findCacheIndexByPackageNameVersionRepo(const QString & pkgname,const QString & version,const QString & repo) const;
    QVector<AlpmPackage *> findByPackageNameProvides(const AlpmPackage::Dependence & provide) const;
    QVector<qint64> findCacheIndexesByPackageNameProvides(const AlpmPackage::Dependence & provide) const;

    // result of packages() must be sorted by AlpmPackage::name()
    virtual const QVector<AlpmPackage *> & packages() const = 0;
    virtual const QStringList & groups() const = 0;

protected:
    virtual const QMap<AlpmPackage::Dependence,QVector<qint64> > & provides() const = 0;

private:
    static bool no_version_asc_less(AlpmPackage * item1, AlpmPackage * item2);
    static bool no_version_asc_equal(AlpmPackage * item1, AlpmPackage * item2);
    static bool asc_version_equal(AlpmPackage * item1, AlpmPackage * item2);
};

class AlpmDB : public AlpmAbstractDB {
public:
    AlpmDB(const AlpmDB & db);
    ~AlpmDB();
    bool isValid() const;
    QString name() const;
    bool update(bool force = false);

    AlpmDB & operator=(const AlpmDB &other);
    static const QString extension();

    const QVector<AlpmPackage *> & packages() const;
    const QStringList & groups() const;

protected:
    AlpmDB();
    AlpmDB(alpm_db_t * db_handle);
    const QMap<AlpmPackage::Dependence,QVector<qint64> > & provides() const;

    static QMap <QString,QVector<AlpmPackage *> > m_packages;
    static QMap <QString,QStringList> m_groups;
    static QMap <QString,QMap<AlpmPackage::Dependence,QVector<qint64> > > m_provides;

private:
    static void clean_pkg_cache(const QString & dbname);
    static void clean_pkg_caches();
    template<class T> T & check_error(T & t) const;
    template<class T> T check_error(const T & t) const;
    void check_error() const;

    alpm_db_t * m_db_handle;

    friend class Alpm;
    friend class AlpmPackage;
};

#endif // ALPMDB_H
