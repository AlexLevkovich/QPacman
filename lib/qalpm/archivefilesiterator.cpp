/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "archivefilesiterator.h"
#include <archive.h>
#include <archive_entry.h>
#include <QDebug>
#include <QDir>

#define MAX_PATH 2048

QByteArray ArchiveEntry::entryReadAllRemaining() {
    qint64 size = entrySize();
    QByteArray buff;
    if (size <= 0) return buff;
    qint64 read_count = 0;
    qint64 ret;
    buff.resize(size);
    do {
        ret = entryRead(buff.data()+read_count,BUFSIZ);
        if (ret > 0) read_count += ret;
        else break;
    } while (read_count < size);

    if (read_count < size) buff.resize(read_count);
    return buff;
}

bool ArchiveEntry::entryReadLine(QByteArray & data,qint64 maxSize) {
    if (maxSize > 0) data.reserve(maxSize);
    data.resize(0);
    char ch;
    qint64 ret;
    do {
        if ((ret = entryRead(&ch,1)) < 0) return false;
        if (ret == 0) break;
        data.resize(data.size()+1);
        data[data.size()-1] = ch;
        if (maxSize > 0 && data.size() >= maxSize) break;
    } while (ch != '\n');

    return true;
}

bool ArchiveEntry::entryRead(QByteArray & data,qint64 maxSize) {
    if (maxSize <= 0) {
        data.resize(0);
        return true;
    }
    data.resize(maxSize);
    qint64 ret = entryRead(data.data(),maxSize);
    if (ret < 0) {
        data.resize(0);
        return false;
    }
    if (ret == 0) {
        data.resize(0);
        return true;
    }
    if (ret < maxSize) data.resize(ret);
    return true;
}

ArchiveFileIterator::ArchiveFileIterator(const ArchiveFileIterator &other) {
    *this = other;
}

ArchiveFileIterator::ArchiveFileIterator(ArchiveReader * reader,off_t pos) {
    m_reader = reader;
    if (atEnd() || !m_reader->setPos(pos)) m_reader = nullptr;
    else if (!m_reader->next()) m_reader = nullptr;
}

ArchiveFileReader::ArchiveFileReader(const QString & archive_path) {
    init();

    if((m_archive = archive_read_new()) == nullptr) {
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
    if(archive_read_open_fd(m_archive,m_fd,(buf.st_blksize > BUFSIZ)?buf.st_blksize:BUFSIZ) != ARCHIVE_OK) {
        m_error = QString::fromLocal8Bit(archive_error_string(m_archive));
        close();
        return;
    }
    m_begin = lseek(m_fd,0,SEEK_CUR);
    if (m_begin == (off_t)-1) {
        m_error = QString::fromLocal8Bit(strerror(errno));
        close();
    }
}

void ArchiveFileReader::init() {
    m_fd = -1;
    m_archive = nullptr;
    m_entry = nullptr;
    m_begin = (off_t)-1;
    m_entryname.clear();
}

ArchiveFileReader::~ArchiveFileReader() {
    close();
}

void ArchiveFileReader::close() {
    if (m_entry != nullptr) archive_entry_free(m_entry);
    if (m_archive != nullptr) archive_read_free(m_archive);
    if(m_fd >= 0) ::close(m_fd);
    init();
}

bool ArchiveFileReader::isValid() const {
    return (m_archive != nullptr && m_fd >= 0);
}

qint64 ArchiveFileReader::entryRead(char * data,qint64 maxSize) {
    if (!isValid()) {
        m_error = QObject::tr("ArchiveFileReader is not initialized!");
        return ARCHIVE_FATAL;
    }

    return archive_read_data(m_archive,data,maxSize);
}

bool ArchiveFileReader::next() const {
    ArchiveFileReader * p_this = (ArchiveFileReader *)this;
    p_this->m_entryname.clear();

    if (!isValid()) {
        p_this->m_error = QObject::tr("ArchiveFileReader is not initialized!");
        return false;
    }

    if (m_entry == nullptr) p_this->m_entry = archive_entry_new();
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
    return (m_reader == nullptr);
}

ArchiveFileIterator ArchiveFileReader::begin() const {
    if (!isValid()) return ArchiveFileReader::end();
    return ArchiveFileIterator((ArchiveReader *)this,m_begin);
}

ArchiveFileIterator ArchiveFileReader::end() const {
    return ArchiveFileIterator();
}

ArchiveEntry * ArchiveFileReader::entry() {
    return this;
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
    if (atEnd() || !m_reader->next()) m_reader = nullptr;
    return *this;
}

QString ArchiveFileReader::entryName() const {
    return m_entryname;
}

QString ArchiveFileReader::entrySymLink() const {
    if (m_entry == nullptr || !entryIsSymLink()) return QString();
    return QString::fromLocal8Bit(archive_entry_symlink(m_entry));
}

QDateTime ArchiveFileReader::entryModificationDate() const {
    if (m_entry == nullptr) return QDateTime();
    return QDateTime::fromSecsSinceEpoch(archive_entry_mtime(m_entry));
}

QDateTime ArchiveFileReader::entryLastAccessDate() const {
    if (m_entry == nullptr) return QDateTime();
    return QDateTime::fromSecsSinceEpoch(archive_entry_atime(m_entry));
}

QDateTime ArchiveFileReader::entryCreateDate() const {
    if (m_entry == nullptr) return QDateTime();
    return QDateTime::fromSecsSinceEpoch(archive_entry_ctime(m_entry));
}

bool ArchiveFileReader::entryIsDir() const {
    if (m_entry == nullptr) return false;
    return (archive_entry_filetype(m_entry) == AE_IFDIR);
}

bool ArchiveFileReader::entryIsFile() const {
    if (m_entry == nullptr) return false;
    return (archive_entry_filetype(m_entry) == AE_IFREG);
}

bool ArchiveFileReader::entryIsSymLink() const {
    if (m_entry == nullptr) return false;
    return (archive_entry_filetype(m_entry) == AE_IFLNK && (archive_entry_symlink(m_entry) != nullptr));
}

bool ArchiveFileReader::entryIsSocket() const {
    if (m_entry == nullptr) return false;
    return (archive_entry_filetype(m_entry) == AE_IFSOCK);
}

bool ArchiveFileReader::entryIsFIFO() const {
    if (m_entry == nullptr) return false;
    return (archive_entry_filetype(m_entry) == AE_IFIFO);
}

bool ArchiveFileReader::entryIsChar() const {
    if (m_entry == nullptr) return false;
    return (archive_entry_filetype(m_entry) == AE_IFCHR);
}

bool ArchiveFileReader::entryIsBlock() const {
    if (m_entry == nullptr) return false;
    return (archive_entry_filetype(m_entry) == AE_IFBLK);
}

mode_t ArchiveFileReader::entryPerm() const {
    if (m_entry == nullptr) return 0;
    return archive_entry_mode(m_entry);
}

qint64 ArchiveFileReader::entryUid() const {
    if (m_entry == nullptr) return -1;
    return archive_entry_uid(m_entry);
}

qint64 ArchiveFileReader::entryGid() const {
    if (m_entry == nullptr) return -1;
    return archive_entry_gid(m_entry);
}

qint64 ArchiveFileReader::entrySize() const {
    if (m_entry == nullptr) return -1;
    return archive_entry_size(m_entry);
}

ArchiveEntry * ArchiveFileIterator::operator*() const {
    if (atEnd()) return nullptr;
    return m_reader->entry();
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

QString PackageFileReader::entryName() const {
    return QDir::separator()+ArchiveFileReader::entryName();
}

const QStringList PackageFileReader::fileList(const QString & archive_path,bool add_root_slash) {
    QStringList ret;

    for(ArchiveEntry * inst : PackageFileReader(archive_path)) {
        ret.append((add_root_slash?QDir::separator()+QString():QString())+inst->entryName());
    }

    return ret;
}

ArchiveFileReaderLoop::ArchiveFileReaderLoop(ArchiveReader * reader,QObject * parent) : ThreadRun(parent) {
    m_reader = reader;
    QMetaObject::invokeMethod(this,"init",Qt::QueuedConnection);
}

ArchiveFileReaderLoop::~ArchiveFileReaderLoop() {
    if (m_reader != nullptr) delete m_reader;
}

void ArchiveFileReaderLoop::init() {
    bool ok;
    exec<bool>(ok,this,&ArchiveFileReaderLoop::processing);
    deleteLater();
}

ThreadRun::RC ArchiveFileReaderLoop::lastMethodRC() const {
    return ThreadRun::OK;
}

bool ArchiveFileReaderLoop::processing() {
    for(ArchiveEntry * inst : *m_reader) {
        if (isTerminateFlagSet()) break;
        qRegisterMetaType<mode_t>("mode_t");
        QMetaObject::invokeMethod(this,"fileInfo",Qt::QueuedConnection,Q_ARG(QString,inst->entryName()),Q_ARG(qint64,inst->entrySize()),Q_ARG(QString,inst->entrySymLink()),Q_ARG(QDateTime,inst->entryModificationDate()),Q_ARG(mode_t,inst->entryPerm()));
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
    if (Alpm::instance() == nullptr || !Alpm::instance()->isOpen()) {
        m_error = QObject::tr("Alpm is not open!");
        return;
    }
    AlpmDB local_db = Alpm::instance()->localDB();
    AlpmPackage pkg = local_db.findByPackageName(pkg_name);
    if (!pkg.isValid()) {
        m_error = QObject::tr("The package hasn't been found!");
        return;
    }
    m_files = pkg.files();
}

bool InstalledPackageFileReader::next() const {
    ((InstalledPackageFileReader *)this)->m_stat.st_size = -1;
    if ((m_index + 1) >= m_files.count()) return false;
    ((InstalledPackageFileReader *)this)->m_index++;
    ((InstalledPackageFileReader *)this)->m_entry_file.close();
    if (entryIsFile() || entryIsSymLink()) {
        ((InstalledPackageFileReader *)this)->m_entry_file.setFileName(entryName());
        ((InstalledPackageFileReader *)this)->m_entry_file.open(QIODevice::ReadOnly);
    }

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
    return m_files[m_index].path();
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

ArchiveFileIterator InstalledPackageFileReader::begin() const {
    if (m_files.count() <= 0) return ArchiveFileIterator();
    return ArchiveFileIterator((ArchiveReader *)this,0);
}

ArchiveFileIterator InstalledPackageFileReader::end() const {
    return ArchiveFileIterator();
}

off_t InstalledPackageFileReader::pos() const {
    return m_index;
}

bool InstalledPackageFileReader::setPos(off_t pos) {
    m_index = pos;
    return isValid();
}

ArchiveEntry * InstalledPackageFileReader::entry() {
    return this;
}

qint64 InstalledPackageFileReader::entryRead(char * data,qint64 maxSize) {
    if (!isValid()) {
        m_error = QObject::tr("InstalledPackageFileReader is not initialized!");
        return -1;
    }

    if (!m_entry_file.isOpen()) {
        m_error = QObject::tr("InstalledPackageFileReader: entry can't be read!");
        return -1;
    }

    return m_entry_file.read(data,maxSize);
}

const QStringList InstalledPackageFileReader::fileList(const QString & pkg_name) {
    QStringList ret;

    for(ArchiveEntry * inst : InstalledPackageFileReader(pkg_name)) {
        ret.append(inst->entryName());
    }
    return ret;
}
