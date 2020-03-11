/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ARCHIVEFILESITERATOR_H
#define ARCHIVEFILESITERATOR_H

#include <QStringList>
#include <QObject>
#include "alpmfuture.h"

struct archive;
class ArchiveFileReader;

class ArchiveFileIterator {
public:
    ArchiveFileIterator(const ArchiveFileIterator & other);
    bool operator!=(const ArchiveFileIterator& other) const;
    ArchiveFileIterator & operator++();
    QString operator*() const;
    ArchiveFileIterator & operator=(const ArchiveFileIterator &other);

private:
    ArchiveFileIterator(ArchiveFileReader * reader = NULL,off_t pos = (off_t)-1);
    bool atEnd() const;

    ArchiveFileReader * m_reader;
    friend class ArchiveFileReader;
};

class ArchiveFileReader {
public:
    ArchiveFileReader(const QString & archive_path);
    virtual ~ArchiveFileReader();
    bool hasNext() const;
    QString next() const;
    QString entryName() const;
    bool atEnd() const;
    QString errorString() const;

    // std::iterator compability functions
    ArchiveFileIterator begin() const;
    ArchiveFileIterator end() const;

    static const QStringList fileList(const QString & archive_path,bool add_root_slash = false);
protected:
    // returns false by default
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
