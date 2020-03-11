/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "archivefilesiterator.h"
#include <archive.h>
#include <archive_entry.h>
#include <QDebug>
#include <QDir>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef BUFSIZ
#define ALPM_BUFFER_SIZE BUFSIZ
#else
#define ALPM_BUFFER_SIZE 8192
#endif

ArchiveFileIterator::ArchiveFileIterator(const ArchiveFileIterator &other) {
    *this = other;
}

ArchiveFileIterator::ArchiveFileIterator(ArchiveFileReader * reader,off_t pos) {
    m_reader = reader;
    if (atEnd() || !m_reader->setPos(pos)) m_reader = NULL;
    else if (m_reader->hasNext()) m_reader->next();
    else m_reader = NULL;
}

ArchiveFileReader::ArchiveFileReader(const QString & archive_path) {
    init();

    if((m_archive = archive_read_new()) == NULL) {
        m_error = QObject::tr("Cannot init archive's handle!!!");
        return;
    }
#if ARCHIVE_VERSION_NUMBER >= 3000000
    archive_read_support_filter_all(m_archive);
#else
    archive_read_support_compression_all(m_archive);
#endif
    archive_read_support_format_all(m_archive);
    m_fd = ::open(archive_path.toLocal8Bit().constData(),O_RDONLY | O_CLOEXEC);
    if(m_fd < 0) {
        m_error = QString::fromLocal8Bit(strerror(errno));
        close();
        return;
    }

    struct stat buf;
    if(fstat(m_fd,&buf) != 0) {
        m_error = QString::fromLocal8Bit(strerror(errno));
        close();
        return;
    }
    if(archive_read_open_fd(m_archive,m_fd,(buf.st_blksize > ALPM_BUFFER_SIZE)?buf.st_blksize:ALPM_BUFFER_SIZE) != ARCHIVE_OK) {
        m_error = QString::fromLocal8Bit(archive_error_string(m_archive));
        close();
        return;
    }
    m_begin = pos();
    if (m_begin == (off_t)-1) {
        m_error = QString::fromLocal8Bit(strerror(errno));
        close();
    }
}

void ArchiveFileReader::init() {
    m_fd = -1;
    m_archive = NULL;
    m_entry = NULL;
    m_begin = (off_t)-1;
    m_entryname.clear();
}

ArchiveFileReader::~ArchiveFileReader() {
    close();
}

void ArchiveFileReader::close() {
    if (m_archive != NULL) {
    #if ARCHIVE_VERSION_NUMBER >= 3000000
        archive_read_free(m_archive);
    #else
        archive_read_finish(m_archive);
    #endif
    }
    if(m_fd >= 0) ::close(m_fd);
    init();
}

bool ArchiveFileReader::isValid() const {
    return (m_archive != NULL && m_fd >= 0);
}

bool ArchiveFileReader::hasNext() const {
    if (!isValid()) return false;
    bool ret =  (archive_read_next_header(m_archive,(archive_entry **)&m_entry) == ARCHIVE_OK);
    if (!ret) ((ArchiveFileReader *)this)->close();
    return ret;
}

QString ArchiveFileReader::next() const {
    if (!isValid() || (m_entry == NULL)) return QString();

    ArchiveFileReader * p_this = (ArchiveFileReader *)this;
    p_this->m_entryname = QString::fromLocal8Bit(archive_entry_pathname(m_entry));
    if (archive_read_data_skip(m_archive) != ARCHIVE_OK) {
        p_this->m_error = QString::fromLocal8Bit(archive_error_string(m_archive));
        p_this->close();
    }
    if (omitText(m_entryname) && hasNext()) p_this->m_entryname = next();
    return m_entryname;
}

const QStringList ArchiveFileReader::fileList(const QString & archive_path,bool add_root_slash) {
    QStringList ret;

    ArchiveFileReader it(archive_path);
    for(QString filename : it) {
        ret.append((add_root_slash?QDir::separator():QChar())+filename);
    }
    return ret;
}

bool ArchiveFileIterator::atEnd() const {
    return (m_reader == NULL);
}

ArchiveFileIterator ArchiveFileReader::begin() const {
    if (!isValid()) return ArchiveFileReader::end();
    return ArchiveFileIterator((ArchiveFileReader *)this,m_begin);
}

ArchiveFileIterator ArchiveFileReader::end() const {
    return ArchiveFileIterator();
}

bool ArchiveFileReader::atEnd() const {
    return !isValid();
}

off_t ArchiveFileReader::pos() const {
    return lseek(m_fd,0,SEEK_CUR);
}

bool ArchiveFileReader::setPos(off_t pos) {
    if (!isValid()) return false;
    if (lseek(m_fd,pos,SEEK_SET) == (off_t)-1) {
        m_error = QString::fromLocal8Bit(strerror(errno));
        close();
        return false;
    }
    return true;
}

bool ArchiveFileIterator::operator!=(const ArchiveFileIterator& other) const {
    if (atEnd() && other.atEnd()) return false;
    if (atEnd() || other.atEnd()) return true;

    return (m_reader->pos() != other.m_reader->pos());
}

ArchiveFileIterator & ArchiveFileIterator::operator++() {
    if (!atEnd() && m_reader->hasNext()) m_reader->next();
    else m_reader = NULL;
    return *this;
}

QString ArchiveFileReader::entryName() const {
    return m_entryname;
}

QString ArchiveFileIterator::operator*() const {
    if (atEnd()) return QString();
    return m_reader->entryName();
}

ArchiveFileIterator & ArchiveFileIterator::operator=(const ArchiveFileIterator &other) {
    m_reader = other.m_reader;
    return *this;
}

QString ArchiveFileReader::errorString() const {
    return m_error;
}

bool ArchiveFileReader::omitText(const QString &) const {
    return false;
}

PackageFileReader::PackageFileReader(const QString & archive_path) : ArchiveFileReader(archive_path) {}

bool PackageFileReader::omitText(const QString & path) const {
    return (path == ".MTREE" || path == ".BUILDINFO" || path == ".PKGINFO");
}

const QStringList PackageFileReader::fileList(const QString & archive_path,bool add_root_slash) {
    QStringList ret;

    PackageFileReader reader(archive_path);
    for(QString filename : reader) {
        ret.append((add_root_slash?QDir::separator():QChar())+filename);
    }

    return ret;
}

ArchiveFileReaderLoop::ArchiveFileReaderLoop(ArchiveFileReader * reader,bool add_root_slash,QObject * parent) : ThreadRun(parent) {
    m_reader = reader;
    m_add_root_slash = add_root_slash;
    QMetaObject::invokeMethod(this,"init",Qt::QueuedConnection);
}

ArchiveFileReaderLoop::~ArchiveFileReaderLoop() {
    if (m_reader != NULL) delete m_reader;
}

void ArchiveFileReaderLoop::init() {
    bool ok;
    run<bool>(ok,this,&ArchiveFileReaderLoop::processing,m_add_root_slash);
    deleteLater();
}

bool ArchiveFileReaderLoop::processing(bool add_root_slash) {
    for(QString filename : *m_reader) {
        if (isTerminateFlagSet()) break;
        QMetaObject::invokeMethod(this,"filePath",Qt::QueuedConnection,Q_ARG(QString,(add_root_slash?QDir::separator():QChar())+filename));
    }
    if (!m_reader->errorString().isEmpty()) {
        QMetaObject::invokeMethod(this,"error",Qt::QueuedConnection,Q_ARG(QString,m_reader->errorString()));
        return false;
    }
    return true;
}
