/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "archivefilesiterator.h"
#include <archive.h>
#include <archive_entry.h>
#include <QDebug>
#include <QDir>
#include "libalpm.h"

#ifdef BUFSIZ
#define ALPM_BUFFER_SIZE BUFSIZ
#else
#define ALPM_BUFFER_SIZE 8192
#endif

#define MAX_PATH 2048

ArchiveFileIterator::ArchiveFileIterator(const ArchiveFileIterator &other) {
    *this = other;
}

ArchiveFileIterator::ArchiveFileIterator(ArchiveFileReader * reader,off_t pos) {
    m_reader = reader;
    if (atEnd() || !m_reader->setPos(pos)) m_reader = NULL;
    else if (!m_reader->next()) m_reader = NULL;
}

ArchiveFileReader::ArchiveFileReader(const QString & archive_path) {
    init();

    if((m_archive = archive_read_new()) == NULL) {
        m_error = QObject::tr("Cannot init archive's handle!!!");
        return;
    }
    archive_read_support_filter_all(m_archive);
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
    if (m_entry != NULL) archive_entry_free(m_entry);
    if (m_archive != NULL) archive_read_free(m_archive);
    if(m_fd >= 0) ::close(m_fd);
    init();
}

bool ArchiveFileReader::isValid() const {
    return (m_archive != NULL && m_fd >= 0);
}

bool ArchiveFileReader::next() const {
    ArchiveFileReader * p_this = (ArchiveFileReader *)this;
    p_this->m_entryname.clear();

    if (!isValid()) return false;

    if (m_entry == NULL) p_this->m_entry = archive_entry_new();
    if (archive_read_next_header2(m_archive,m_entry) != ARCHIVE_OK) {
        p_this->m_error = QString::fromLocal8Bit(archive_error_string(m_archive));
        p_this->close();
        return false;
    }

    p_this->m_entryname = QString::fromLocal8Bit(archive_entry_pathname(m_entry));

    if (omitText(m_entryname)) return next();
    return true;
}

const QStringList ArchiveFileReader::fileList(const QString & archive_path,bool add_root_slash) {
    QStringList ret;

    for(ArchiveEntry * inst : ArchiveFileReader(archive_path)) {
        ret.append((add_root_slash?QDir::separator():QChar())+inst->entryName());
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

    return ((m_reader->pos() != other.m_reader->pos()) || (m_reader != other.m_reader));
}

ArchiveFileIterator & ArchiveFileIterator::operator++() {
    if (atEnd() || !m_reader->next()) m_reader = NULL;
    return *this;
}

QString ArchiveFileReader::entryName() const {
    return m_entryname;
}

QString ArchiveFileReader::entrySymLink() const {
    if (m_entry == NULL || !entryIsSymLink()) return QString();
    return QString::fromLocal8Bit(archive_entry_symlink(m_entry));
}

QDateTime ArchiveFileReader::entryModificationDate() const {
    if (m_entry == NULL) return QDateTime();
    return QDateTime::fromSecsSinceEpoch(archive_entry_mtime(m_entry));
}

QDateTime ArchiveFileReader::entryLastAccessDate() const {
    if (m_entry == NULL) return QDateTime();
    return QDateTime::fromSecsSinceEpoch(archive_entry_atime(m_entry));
}

QDateTime ArchiveFileReader::entryCreateDate() const {
    if (m_entry == NULL) return QDateTime();
    return QDateTime::fromSecsSinceEpoch(archive_entry_ctime(m_entry));
}

bool ArchiveFileReader::entryIsDir() const {
    if (m_entry == NULL) return false;
    return (archive_entry_filetype(m_entry) == AE_IFDIR);
}

bool ArchiveFileReader::entryIsFile() const {
    if (m_entry == NULL) return false;
    return (archive_entry_filetype(m_entry) == AE_IFREG);
}

bool ArchiveFileReader::entryIsSymLink() const {
    if (m_entry == NULL) return false;
    return (archive_entry_filetype(m_entry) == AE_IFLNK && (archive_entry_symlink(m_entry) != NULL));
}

bool ArchiveFileReader::entryIsSocket() const {
    if (m_entry == NULL) return false;
    return (archive_entry_filetype(m_entry) == AE_IFSOCK);
}

bool ArchiveFileReader::entryIsFIFO() const {
    if (m_entry == NULL) return false;
    return (archive_entry_filetype(m_entry) == AE_IFIFO);
}

bool ArchiveFileReader::entryIsChar() const {
    if (m_entry == NULL) return false;
    return (archive_entry_filetype(m_entry) == AE_IFCHR);
}

bool ArchiveFileReader::entryIsBlock() const {
    if (m_entry == NULL) return false;
    return (archive_entry_filetype(m_entry) == AE_IFBLK);
}

mode_t ArchiveFileReader::entryPerm() const {
    if (m_entry == NULL) return 0;
    return archive_entry_perm(m_entry);
}

qint64 ArchiveFileReader::entryUid() const {
    if (m_entry == NULL) return -1;
    return archive_entry_uid(m_entry);
}

qint64 ArchiveFileReader::entryGid() const {
    if (m_entry == NULL) return -1;
    return archive_entry_gid(m_entry);
}

qint64 ArchiveFileReader::entrySize() const {
    if (m_entry == NULL) return -1;
    return archive_entry_size(m_entry);
}

ArchiveEntry * ArchiveFileIterator::operator*() const {
    if (atEnd()) return NULL;
    return m_reader;
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

    for(ArchiveEntry * inst : PackageFileReader(archive_path)) {
        ret.append((add_root_slash?QDir::separator():QChar())+inst->entryName());
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
    for(ArchiveEntry * inst : *m_reader) {
        if (isTerminateFlagSet()) break;
        QMetaObject::invokeMethod(this,"filePath",Qt::QueuedConnection,Q_ARG(QString,(add_root_slash?QDir::separator():QChar())+inst->entryName()));
    }
    if (!m_reader->errorString().isEmpty()) {
        QMetaObject::invokeMethod(this,"error",Qt::QueuedConnection,Q_ARG(QString,m_reader->errorString()));
        return false;
    }
    return true;
}

InstalledPackageFileReader::InstalledPackageFileReader(const QString & pkg_name) {
    m_index = -1;
    m_stat.st_size = -1;
    if (Alpm::instance() == NULL || !Alpm::instance()->isOpen()) {
        m_error = QObject::tr("Alpm is not open!");
        return;
    }
    AlpmDB local_db = Alpm::instance()->localDB();
    QVector<AlpmPackage *> pkgs = local_db.findByPackageName(pkg_name);
    if (pkgs.count() <= 0) {
        m_error = QObject::tr("The package has been not found!");
        return;
    }
    m_files = pkgs[0]->files();
}

bool InstalledPackageFileReader::next() const {
    ((InstalledPackageFileReader *)this)->m_stat.st_size = -1;
    if ((m_index + 1) >= m_files.count()) return false;
    ((InstalledPackageFileReader *)this)->m_index++;
    return true;
}

bool InstalledPackageFileReader::isValid() const {
    return ((m_index >= 0) && (m_index < m_files.count()));
}

bool InstalledPackageFileReader::stat() const {
    if (m_stat.st_size != -1) return true;

    if (!isValid()) {
        ((InstalledPackageFileReader *)this)->m_error = QObject::tr("Invalid entry name for stat()!");
        return false;
    }

    if (::stat(entryName().toLocal8Bit().constData(),&((InstalledPackageFileReader *)this)->m_stat) != 0) {
        ((InstalledPackageFileReader *)this)->m_error = strerror(errno);
        return false;
    }
    return true;
}

QString InstalledPackageFileReader::entryName() const {
    if (!isValid()) return QString();
    return m_files[m_index];
}

QString InstalledPackageFileReader::entrySymLink() const {
    if (!isValid() || !entryIsSymLink()) return QString();
    char link_path[MAX_PATH];
    if (::readlink(entryName().toLocal8Bit().constData(),link_path,MAX_PATH-1) < 0) {
        ((InstalledPackageFileReader *)this)->m_error = strerror(errno);
        return QString();
    }
    return QString::fromLocal8Bit(link_path);
}

QDateTime InstalledPackageFileReader::entryModificationDate() const {
    if (!isValid() || !stat()) return QDateTime();
    return QDateTime::fromSecsSinceEpoch(m_stat.st_mtime);
}

QDateTime InstalledPackageFileReader::entryLastAccessDate() const {
    if (!isValid() || !stat()) return QDateTime();
    return QDateTime::fromSecsSinceEpoch(m_stat.st_atime);
}

QDateTime InstalledPackageFileReader::entryCreateDate() const {
    if (!isValid() || !stat()) return QDateTime();
    return QDateTime::fromSecsSinceEpoch(m_stat.st_ctime);
}

bool InstalledPackageFileReader::entryIsDir() const {
    if (!isValid() || !stat()) return false;
    return S_ISDIR(m_stat.st_mode);
}

bool InstalledPackageFileReader::entryIsFile() const {
    if (!isValid() || !stat()) return false;
    return S_ISREG(m_stat.st_mode);
}

bool InstalledPackageFileReader::entryIsSymLink() const {
    if (!isValid() || !stat()) return false;
    return S_ISLNK(m_stat.st_mode);
}

bool InstalledPackageFileReader::entryIsSocket() const {
    if (!isValid() || !stat()) return false;
    return S_ISSOCK(m_stat.st_mode);
}

bool InstalledPackageFileReader::entryIsFIFO() const {
    if (!isValid() || !stat()) return false;
    return S_ISFIFO(m_stat.st_mode);
}

bool InstalledPackageFileReader::entryIsChar() const {
    if (!isValid() || !stat()) return false;
    return S_ISCHR(m_stat.st_mode);
}

bool InstalledPackageFileReader::entryIsBlock() const {
    if (!isValid() || !stat()) return false;
    return S_ISBLK(m_stat.st_mode);
}

mode_t InstalledPackageFileReader::entryPerm() const {
    if (!isValid() || !stat()) return 0;
    return m_stat.st_mode;
}

qint64 InstalledPackageFileReader::entryUid() const {
    if (!isValid() || !stat()) return -1;
    return m_stat.st_uid;
}

qint64 InstalledPackageFileReader::entryGid() const {
    if (!isValid() || !stat()) return -1;
    return m_stat.st_gid;
}

qint64 InstalledPackageFileReader::entrySize() const {
    if (!isValid() || !stat()) return 0;
    return m_stat.st_size;
}

bool InstalledPackageFileReader::atEnd() const {
    return !isValid();
}

QString InstalledPackageFileReader::errorString() const {
    return m_error;
}

LocalFileIterator InstalledPackageFileReader::begin() const {
    if (!isValid()) return LocalFileIterator();
    return LocalFileIterator((InstalledPackageFileReader *)this);
}

LocalFileIterator InstalledPackageFileReader::end() const {
    return LocalFileIterator();
}

LocalFileIterator::LocalFileIterator(const LocalFileIterator & other) {
    *this = other;
}

LocalFileIterator::LocalFileIterator(InstalledPackageFileReader * reader) {
    m_reader = reader;
}

bool LocalFileIterator::operator!=(const LocalFileIterator& other) const {
    if (atEnd() && other.atEnd()) return false;
    if (atEnd() || other.atEnd()) return true;

    return ((m_reader->m_index != other.m_reader->m_index) || (m_reader != other.m_reader));
}

LocalFileIterator & LocalFileIterator::operator++() {
    if (atEnd() || !m_reader->next()) m_reader = NULL;
    return *this;
}

ArchiveEntry * LocalFileIterator::operator*() const {
    if (atEnd()) return NULL;
    return m_reader;
}

LocalFileIterator & LocalFileIterator::operator=(const LocalFileIterator &other) {
    m_reader = other.m_reader;
    return *this;
}

bool LocalFileIterator::atEnd() const {
    return (m_reader == NULL);
}

const QStringList InstalledPackageFileReader::fileList(const QString & pkg_name) {
    QStringList ret;

    for(ArchiveEntry * inst : InstalledPackageFileReader(pkg_name)) {
        ret.append(inst->entryName());
    }
    return ret;
}
