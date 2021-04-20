#ifndef SEARCHPACKAGE_H
#define SEARCHPACKAGE_H

#include <alpm.h>
#include <archive.h>
#include <archive_entry.h>
#include <QLatin1String>
#include <QString>

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
