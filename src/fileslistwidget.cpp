/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "fileslistwidget.h"
#include "byteshumanizer.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include "static.h"
#include "libalpm.h"
#include "themeicons.h"
#include "archivefilesiterator.h"
#include "waitindicator.h"
#include "slottedeventloop.h"

FilesListWidget::FilesListWidget(QWidget *parent) : QTreeWidget(parent) {
    folderIcon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    fileIcon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    m_focusPolicy = focusPolicy();
    setMouseTracking(true);
    m_pkg = NULL;
    reader = NULL;
    wait_ind = NULL;

    in_rect = false;
    m_do_refresh_picture = true;
    setHeaderHidden(true);
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::NoSelection);
    m_refresh_pix.load(ThemeIcons::name(ThemeIcons::REFRESH_BTN));
    m_refresh_shadow_pix.load(ThemeIcons::name(ThemeIcons::REFRESH_BTN_SHADOW));
    pix = m_refresh_pix;
    shadow_pix = m_refresh_shadow_pix;

    qRegisterMetaType<AlpmPackage *>("AlpmPackage *");
}

void FilesListWidget::refill() {
    if (m_pkg == NULL) {
        clear();
        return;
    }

    fill(m_pkg);
}

void FilesListWidget::fill(AlpmPackage * pkg) {
    clear();
    update();

    m_pkg = pkg;
    m_files.clear();
    if (!Alpm::isOpen()) return;

    if (reader != NULL) {
        reader->setTerminateFlag();
        QMetaObject::invokeMethod(this,"refill",Qt::QueuedConnection);
        return;
    }

    QString pkg_cache_path;
    if (!pkg->isInstalled() && !pkg->isDownloaded(&pkg_cache_path)) {
        reader = new ArchiveFileReaderLoop(new InstalledPackageFileReader(pkg->name()));
    }
    else if (!pkg->isInstalled() && pkg->isDownloaded(&pkg_cache_path) && !pkg_cache_path.isEmpty()) {
        reader = new ArchiveFileReaderLoop(new PackageFileReader(pkg_cache_path));
    }
    else if (pkg->isInstalled()) reader = new ArchiveFileReaderLoop(new InstalledPackageFileReader(pkg->name()));

    connect(reader,SIGNAL(fileInfo(const QString &,qint64,const QString &,const QDateTime &,mode_t)),this,SLOT(newFileEntry(const QString &,qint64,const QString &,const QDateTime &,mode_t)));
    connect(reader,SIGNAL(destroyed()),SLOT(readerDestroyed()));
}

void FilesListWidget::readerDestroyed() {
    if (wait_ind != NULL) delete wait_ind;
    wait_ind = NULL;
    if (!reader->isTerminateFlagSet()) {
        reader = NULL;
        if (m_files.isEmpty()) {
            m_do_refresh_picture = (!m_pkg->isInstalled() && !m_pkg->isDownloaded(NULL));
            setHeaderHidden(m_do_refresh_picture);
            update();
            setSelectionMode(m_do_refresh_picture?QAbstractItemView::NoSelection:QAbstractItemView::SingleSelection);
            return;
        }
        m_do_refresh_picture = false;
        setHeaderHidden(false);
        setSelectionMode(QAbstractItemView::SingleSelection);
        fill();
    }
    reader = NULL;
}

void FilesListWidget::newFileEntry(const QString & name,qint64 size,const QString & linkContents,const QDateTime & mdate,mode_t perms) {
    if (wait_ind == NULL) {
        m_do_refresh_picture = false;
        setHeaderHidden(true);
        update();
        (wait_ind = new WaitIndicator(this->viewport()))->start();
    }
    QFileInfo fi(name);
    if (fi.dir().path() == ".") {
        m_files["/"].append(FilesListWidget::FileEntry(name,size,linkContents,mdate,perms));
    }
    else {
        m_files[fi.dir().path()].append(FilesListWidget::FileEntry(fi.fileName(),size,linkContents,mdate,perms));
    }
}

void FilesListWidget::fill() {
    QString prev_key;
    QString key;
    QList<FileEntry> values;
    QMapIterator<QString,QList<FileEntry> > i(m_files);
    while (i.hasNext()) {
        i.next();
        key = i.key();
        if (!prev_key.isEmpty() && ((m_files[prev_key].count() <= 0) || ((m_files[prev_key].count() == 1) && (m_files[prev_key])[0].name().isEmpty())) && key.contains(prev_key)) m_files.remove(prev_key);
        prev_key = key;
    }

    i = QMapIterator<QString,QList<FileEntry> >(m_files);
    while (i.hasNext()) {
        i.next();
        QTreeWidgetItem * item = new QTreeWidgetItem(this);
        item->setText(0,i.key());
        item->setIcon(0,folderIcon);
        values = i.value();
        for (int j=0;j<values.count();j++) {
            if (!values[j].name().isEmpty()) {
                QTreeWidgetItem * child_item = new QTreeWidgetItem(item);
                child_item->setText(0,values[j].name());
                child_item->setIcon(0,fileIcon);
                child_item->setText(1,values[j].size());
                child_item->setText(2,values[j].perm());
                child_item->setText(3,values[j].mdate());
            }
        }
    }

    expandAll();
    resizeColumnToContents(0);
}

void FilesListWidget::paintEvent(QPaintEvent *event) {
    QTreeWidget::paintEvent(event);
    if (!m_do_refresh_picture) return;

    QPainter painter(viewport());
    if (event->rect().intersects(drawRect)) painter.drawPixmap(pt,in_rect?shadow_pix:pix);
    if (event->rect().intersects(textRect)) painter.drawText(textRect,Qt::AlignHCenter|Qt::AlignVCenter,tr("Download the package and update..."));
}

void FilesListWidget::resizeEvent(QResizeEvent * event) {
    QTreeWidget::resizeEvent(event);

    int font_height = QFontMetrics(font()).height();
    int needed_pix_height = event->size().height()-10-font_height;
    pix = m_refresh_pix;
    shadow_pix = m_refresh_shadow_pix;
    if (m_refresh_pix.height() > needed_pix_height) {
        pix = m_refresh_pix.scaledToHeight(needed_pix_height,Qt::SmoothTransformation);
        shadow_pix = m_refresh_shadow_pix.scaledToHeight(needed_pix_height,Qt::SmoothTransformation);
    }
    else {
        pix = m_refresh_pix;
        shadow_pix = m_refresh_shadow_pix;
    }
    pt = QPoint((event->size().width()-pix.width())/2,(event->size().height()-(pix.height()+10+font_height))/2);
    drawRect = QRect(pt,QSize(pix.width(),pix.height()));
    textRect = QRect(0,pt.y()+pix.height(),width(),10+font_height);
    viewport()->update();
}

void FilesListWidget::mouseMoveEvent(QMouseEvent *event) {
    QTreeWidget::mouseMoveEvent(event);

    if (drawRect.contains(event->pos()) && !in_rect) {
        in_rect = true;
        viewport()->update();
    }
    if (!drawRect.contains(event->pos()) && in_rect) {
        in_rect = false;
        viewport()->update();
    }
}

bool FilesListWidget::event(QEvent *event) {
    if (event->type() == QEvent::HoverLeave) {
        in_rect = false;
        viewport()->update();
    }
    return QTreeWidget::event(event);
}

void FilesListWidget::mousePressEvent(QMouseEvent *event) {
    QTreeWidget::mousePressEvent(event);
    if (!m_do_refresh_picture) return;

    if (in_rect && (m_pkg != NULL) && !m_pkg->isDownloaded(NULL)) emit downloadRequested(m_pkg);
    else if (m_pkg->isDownloaded(NULL)) refill();
}

static const QString strmode(int mode) {
    QByteArray arr(11,' ');
    char * p = arr.data();
     /* print type */
    switch (mode & S_IFMT) {
    case S_IFDIR:			/* directory */
        *p++ = 'd';
        break;
    case S_IFCHR:			/* character special */
        *p++ = 'c';
        break;
    case S_IFBLK:			/* block special */
        *p++ = 'b';
        break;
    case S_IFREG:			/* regular */
        *p++ = '-';
        break;
    case S_IFLNK:			/* symbolic link */
        *p++ = 'l';
        break;
#ifdef S_IFSOCK
    case S_IFSOCK:			/* socket */
        *p++ = 's';
        break;
#endif
#ifdef S_IFIFO
    case S_IFIFO:			/* fifo */
        *p++ = 'p';
        break;
#endif
    default:			/* unknown */
        *p++ = '?';
        break;
    }
    /* usr */
    if (mode & S_IRUSR)
        *p++ = 'r';
    else
        *p++ = '-';
    if (mode & S_IWUSR)
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (S_IXUSR | S_ISUID)) {
    case 0:
        *p++ = '-';
        break;
    case S_IXUSR:
        *p++ = 'x';
        break;
    case S_ISUID:
        *p++ = 'S';
        break;
    case S_IXUSR | S_ISUID:
        *p++ = 's';
        break;
    }
    /* group */
    if (mode & S_IRGRP)
        *p++ = 'r';
    else
        *p++ = '-';
    if (mode & S_IWGRP)
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (S_IXGRP | S_ISGID)) {
    case 0:
        *p++ = '-';
        break;
    case S_IXGRP:
        *p++ = 'x';
        break;
    case S_ISGID:
        *p++ = 'S';
        break;
    case S_IXGRP | S_ISGID:
        *p++ = 's';
        break;
    }
    /* other */
    if (mode & S_IROTH)
        *p++ = 'r';
    else
        *p++ = '-';
    if (mode & S_IWOTH)
        *p++ = 'w';
    else
        *p++ = '-';
    switch (mode & (S_IXOTH | S_ISVTX)) {
    case 0:
        *p++ = '-';
        break;
    case S_IXOTH:
        *p++ = 'x';
        break;
    case S_ISVTX:
        *p++ = 'T';
        break;
    case S_IXOTH | S_ISVTX:
        *p++ = 't';
        break;
    }
    return QString::fromLatin1(arr);
}

FilesListWidget::FileEntry::FileEntry(const QString & name,qint64 size,const QString & linkContents,const QDateTime & mdate,mode_t perms) {
    m_name = name;
    m_size = BytesHumanizer(size).toString();
    m_linkContents = linkContents;
    m_mdate = mdate.toString("ddd MMM dd yyyy hh:mm:ss");
    m_perms = strmode(perms);
}
