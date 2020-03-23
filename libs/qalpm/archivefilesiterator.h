/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ARCHIVEFILESITERATOR_H
#define ARCHIVEFILESITERATOR_H

#include <QStringList>
#include <QObject>
#include <QDateTime>
#include "alpmfuture.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct archive;
class ArchiveFileReader;
class InstalledPackageFileReader;
class ArchiveEntry;

class ArchiveFileIterator {
public:
    ArchiveFileIterator(const ArchiveFileIterator & other);
    bool operator!=(const ArchiveFileIterator& other) const;
    ArchiveFileIterator & operator++();
    ArchiveEntry * operator*() const;
    ArchiveFileIterator & operator=(const ArchiveFileIterator &other);

private:
    ArchiveFileIterator(ArchiveFileReader * reader = NULL,off_t pos = (off_t)-1);
    bool atEnd() const;

    ArchiveFileReader * m_reader;
    friend class ArchiveFileReader;
};

class ArchiveEntry {
public:
    virtual QString entryName() const = 0;
    virtual QString entrySymLink() const = 0;
    virtual QDateTime entryModificationDate() const = 0;
    virtual QDateTime entryLastAccessDate() const = 0;
    virtual QDateTime entryCreateDate() const = 0;
    virtual bool entryIsDir() const = 0;
    virtual bool entryIsFile() const = 0;
    virtual bool entryIsSymLink() const = 0;
    virtual bool entryIsSocket() const = 0;
    virtual bool entryIsFIFO() const = 0;
    virtual bool entryIsChar() const = 0;
    virtual bool entryIsBlock() const = 0;
    virtual mode_t entryPerm() const = 0;
    virtual qint64 entryUid() const = 0;
    virtual qint64 entryGid() const = 0;
    virtual qint64 entrySize() const = 0;
};

class ArchiveFileReader : public ArchiveEntry {
public:
    ArchiveFileReader(const QString & archive_path);
    virtual ~ArchiveFileReader();
    bool next() const;

    QString entryName() const;
    QString entrySymLink() const;
    QDateTime entryModificationDate() const;
    QDateTime entryLastAccessDate() const;
    QDateTime entryCreateDate() const;
    bool entryIsDir() const;
    bool entryIsFile() const;
    bool entryIsSymLink() const;
    bool entryIsSocket() const;
    bool entryIsFIFO() const;
    bool entryIsChar() const;
    bool entryIsBlock() const;
    mode_t entryPerm() const;
    qint64 entryUid() const;
    qint64 entryGid() const;
    qint64 entrySize() const;

    bool atEnd() const;
    QString errorString() const;

    // std::iterator compability functions
    ArchiveFileIterator begin() const;
    ArchiveFileIterator end() const;

    static const QStringList fileList(const QString & archive_path,bool add_root_slash = false);
protected:
    // returns false by default, it might be used to omit some files during iteration
    virtual bool omitText(const QString & path) const;

private:
    void close();
    bool isValid() const;
    off_t pos() const;
    bool setPos(off_t pos);
    void init();

    QString m_entryname;
    archive * m_archive;
    int m_fd;
    off_t m_begin;
    struct archive_entry * m_entry;
    QString m_error;

    friend class ArchiveFileIterator;
};

class PackageFileReader : public ArchiveFileReader {
public:
    PackageFileReader(const QString & archive_path);
    static const QStringList fileList(const QString & archive_path,bool add_root_slash = false);
protected:
    bool omitText(const QString & path) const;
};

class LocalFileIterator {
public:
    LocalFileIterator(const LocalFileIterator & other);
    bool operator!=(const LocalFileIterator& other) const;
    LocalFileIterator & operator++();
    ArchiveEntry * operator*() const;
    LocalFileIterator & operator=(const LocalFileIterator &other);

private:
    LocalFileIterator(InstalledPackageFileReader * reader = NULL);
    bool atEnd() const;

    InstalledPackageFileReader * m_reader;
    friend class InstalledPackageFileReader;
};

class InstalledPackageFileReader : public ArchiveEntry {
public:
    InstalledPackageFileReader(const QString & pkg_name);
    bool next() const;

    QString entryName() const;
    QString entrySymLink() const;
    QDateTime entryModificationDate() const;
    QDateTime entryLastAccessDate() const;
    QDateTime entryCreateDate() const;
    bool entryIsDir() const;
    bool entryIsFile() const;
    bool entryIsSymLink() const;
    bool entryIsSocket() const;
    bool entryIsFIFO() const;
    bool entryIsChar() const;
    bool entryIsBlock() const;
    mode_t entryPerm() const;
    qint64 entryUid() const;
    qint64 entryGid() const;
    qint64 entrySize() const;

    bool atEnd() const;
    QString errorString() const;

    // std::iterator compability functions
    LocalFileIterator begin() const;
    LocalFileIterator end() const;

    static const QStringList fileList(const QString & pkg_name);

private:
    bool isValid() const;
    bool stat() const;

    qint64 m_index;
    QStringList m_files;
    QString m_error;
    struct stat m_stat;

    friend class LocalFileIterator;
};

class ArchiveFileReaderLoop : public ThreadRun {
    Q_OBJECT
public:
    // reader has to be initialized as `new ArchiveFileReader` or `new PackageFileReader`
    // reader will be deleted automatically as well as ArchiveFileReaderLoop
    // so all do you need is `new ArchiveFileReaderLoop()`
    // connect to filePath() and this->destroyed() signal indicates completing the processing
    ArchiveFileReaderLoop(ArchiveFileReader * reader,bool add_root_slash = true,QObject * parent = NULL);
    ~ArchiveFileReaderLoop();
signals:
    void filePath(const QString & path);
    void error(const QString & path);
private slots:
    void init();
private:
    bool processing(bool add_root_slash);

    ArchiveFileReader * m_reader;
    bool m_add_root_slash;
};

#endif // ARCHIVEFILESITERATOR_H
