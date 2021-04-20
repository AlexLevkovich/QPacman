#include "searchpackage.h"
#include "libalpm.h"
#include <QFile>
#include <QDateTime>
#include <QByteArray>
#include <QBuffer>
#include <QTemporaryFile>

#define PKG_DIR "/dev/shm"
#define PKGINFO_NAME ".PKGINFO"

SearchPackage::SearchPackage(const QString & name,const QString & version) {
    init(name.toLatin1().constData(),version.toLatin1().constData());
}

SearchPackage::SearchPackage(const QLatin1String & name,const QLatin1String & version) {
    init(name.latin1(),version.latin1());
}

SearchPackage::SearchPackage(const char * name,const char * version) {
    init(name,version);
}

void SearchPackage::init(const char * name,const char * version) {
    m_handle = NULL;
    m_autofree = true;
    a = NULL;
    if (Alpm::instance() == NULL || Alpm::instance()->m_alpm_handle == NULL) return;

    struct archive_entry *entry;
    char buff[8192];
    qint64 len;
    qint64 wlen;

    a = archive_write_new();
    if (a == NULL) return;
    if (archive_write_add_filter_xz(a) != ARCHIVE_OK) return;
    if (archive_write_set_format_pax_restricted(a)!= ARCHIVE_OK) return;
    QTemporaryFile * tfile = new QTemporaryFile(PKG_DIR"/temp_qpacman_pkg");
    tfile->setAutoRemove(true);
    if (!tfile->open()) return;
    m_filename = tfile->fileName();
    delete tfile;
    if (archive_write_open_filename(a,m_filename.toLocal8Bit().constData()) != ARCHIVE_OK) return;
    entry = archive_entry_new();
    if (entry == NULL) return;
    archive_entry_set_pathname(entry,PKGINFO_NAME);

    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    buffer.write(QByteArray("pkgname = ")+name+"\n");
    buffer.write(QByteArray("pkgbase = ")+name+"\n");
    buffer.write(QByteArray("pkgver = ")+version+"\n");
    buffer.write("pkgdesc = temporary package\n");
    buffer.write("url = https://unknown\n");
    buffer.write(QByteArray("builddate = ")+QByteArray::number(QDateTime::currentSecsSinceEpoch())+"\n");
    buffer.write("packager = Unknown Packager\n");
    buffer.write("size = 0\n");
    buffer.write("arch = any\n");
    buffer.write("license = GPL\n");
    buffer.seek(0);
    archive_entry_set_size(entry,buffer.size());
    archive_entry_set_filetype(entry,AE_IFREG);
    archive_entry_set_perm(entry,0644);
    if (archive_write_header(a,entry) != ARCHIVE_OK) return;
    while ((len = buffer.read(buff,sizeof(buff))) > 0) {
       wlen = archive_write_data(a,buff,len);
       if (wlen <= 0) return;
       if (wlen < len) buffer.seek(buffer.pos()-(len-wlen));
    }
    buffer.close();
    archive_entry_free(entry);
    archive_write_close(a);
    archive_write_free(a);
    a = NULL;

    alpm_pkg_load(Alpm::instance()->m_alpm_handle,m_filename.toLocal8Bit().constData(),1,0,&m_handle);
}

SearchPackage::~SearchPackage() {
    if (a != NULL) {
        archive_write_close(a);
        archive_write_free(a);
    }
    QFile(m_filename).remove();
    if ((m_handle != NULL) && m_autofree) alpm_pkg_free(m_handle);
}

bool SearchPackage::autoFree() const {
    return m_autofree;
}

void SearchPackage::setAutoFree(bool flag) {
    m_autofree = flag;
}

alpm_pkg_t * SearchPackage::packageHandle() {
    return m_handle;
}
