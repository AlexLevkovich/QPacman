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
#include "libalpm.h"

struct archive;
class ArchiveReader;
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
    ArchiveFileIterator(ArchiveReader * reader = NULL,off_t pos = (off_t)-1);
    bool atEnd() const;

    ArchiveReader * m_reader;

    friend class ArchiveFileReader;
    friend class InstalledPackageFileReader;
};

class ArchiveReader {
public:
    virtual ~ArchiveReader() {}

    virtual ArchiveEntry * entry() = 0;
    virtual bool next() const = 0;
    virtual bool atEnd() const = 0;
    virtual QString errorString() const = 0;
    virtual off_t pos() const = 0;
    virtual bool setPos(off_t pos) = 0;

    // std::iterator compability functions
    virtual ArchiveFileIterator begin() const = 0;
    virtual ArchiveFileIterator end() const = 0;
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

class ArchiveFileReader : public ArchiveEntry, public ArchiveReader {
public:
    ArchiveFileReader(const QString & archive_path);
    virtual ~ArchiveFileReader();
    bool next() const;

    virtual QString entryName() const;
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
    ArchiveEntry * entry();

    QString m_entryname;
    archive * m_archive;
    int m_fd;
    off_t m_begin;
    struct archive_entry * m_entry;
    QString m_error;
};

class PackageFileReader : public ArchiveFileReader {
public:
    PackageFileReader(const QString & archive_path);
    QString entryName() const;
    static const QStringList fileList(const QString & archive_path,bool add_root_slash = false);
protected:
    bool omitText(const QString & path) const;
};

class InstalledPackageFileReader : public ArchiveEntry, public ArchiveReader {
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
    ArchiveFileIterator begin() const;
    ArchiveFileIterator end() const;

    static const QStringList fileList(const QString & pkg_name);

private:
    bool isValid() const;
    bool stat() const;
    off_t pos() const;
    bool setPos(off_t pos);
    ArchiveEntry * entry();

    off_t m_index;
    QList<AlpmPackage::FileInfo> m_files;
    QString m_error;
    struct stat m_stat;
};

class ArchiveFileReaderLoop : public ThreadRun {
    Q_OBJECT
public:
    // reader has to be initialized as `new ArchiveFileReader` or `new PackageFileReader`
    // reader will be deleted automatically as well as ArchiveFileReaderLoop
    // so all do you need is `new ArchiveFileReaderLoop()`
    // connect to filePath() and this->destroyed() signal indicates completing the processing
    ArchiveFileReaderLoop(ArchiveReader * reader,QObject * parent = NULL);
    ~ArchiveFileReaderLoop();
signals:
    void fileInfo(const QString & name,qint64 size,const QString & linkContents,const QDateTime & mdate,mode_t perms);
    void error(const QString & err);
protected:
    ThreadRun::RC lastMethodRC() const;
private slots:
    void init();
private:
    bool processing();

    ArchiveReader * m_reader;
};
Q_DECLARE_METATYPE(mode_t)

#endif // ARCHIVEFILESITERATOR_H
