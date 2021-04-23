/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef SEARCHPACKAGE_H
#define SEARCHPACKAGE_H

#include <archive.h>
#include <archive_entry.h>
#include <QLatin1String>
#include <QString>

typedef struct __alpm_pkg_t alpm_pkg_t;

class SearchPackage {
public:
    SearchPackage(const QString & name,const QString & version);
    SearchPackage(const QLatin1String & name,const QLatin1String & version);
    SearchPackage(const char * name,const char * version);
    ~SearchPackage();
    alpm_pkg_t * packageHandle();
    bool autoFree() const;
    void setAutoFree(bool flag);
private:
    void init(const char * name,const char * version);

    alpm_pkg_t * m_handle;
    QString m_filename;
    struct archive *a;
    bool m_autofree;
};

#endif // SEARCHPACKAGE_H
