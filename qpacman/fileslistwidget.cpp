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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

FilesListWidget::FilesListWidget(QWidget *parent) : QTreeWidget(parent) {
    folderIcon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    fileIcon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    setMouseTracking(true);

    in_rect = false;
    m_do_refresh_picture = true;
    setHeaderHidden(true);
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::NoSelection);
    m_refresh_pix.load(ThemeIcons::name(ThemeIcons::REFRESH_BTN));
    m_refresh_shadow_pix.load(ThemeIcons::name(ThemeIcons::REFRESH_BTN_SHADOW));
    pix = m_refresh_pix;
    shadow_pix = m_refresh_shadow_pix;
}

void FilesListWidget::refill() {
    if (!m_pkg.isValid()) {
        clear();
        return;
    }

    fill(m_pkg);
}

void FilesListWidget::fill(const AlpmPackage & pkg) {
    clear();
    update();

    m_pkg = pkg;
    m_files.clear();
    for (AlpmPackage::FileInfo & file: pkg.files()) {
        newFileEntry(file);
    }

    if (m_files.isEmpty()) {
        m_do_refresh_picture = (!m_pkg.isInstalled() && !m_pkg.isDownloaded());
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

void FilesListWidget::newFileEntry(AlpmPackage::FileInfo & file) {
    QFileInfo fi(file.path());
    if (fi.dir().path() == "/" && fi.fileName().startsWith(".")) return;
    file.setPath(fi.fileName());
    m_files[fi.dir().path()].append(file);
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

void FilesListWidget::fill() {
    QString prev_key;
    QString key;
    QList<AlpmPackage::FileInfo> values;
    QMapIterator<QString,QList<AlpmPackage::FileInfo> > i(m_files);
    while (i.hasNext()) {
        i.next();
        key = i.key();
        if (!prev_key.isEmpty() && ((m_files[prev_key].count() <= 0) || ((m_files[prev_key].count() == 1) && (m_files[prev_key])[0].path().isEmpty())) && key.contains(prev_key)) m_files.remove(prev_key);
        prev_key = key;
    }

    i = QMapIterator<QString,QList<AlpmPackage::FileInfo> >(m_files);
    while (i.hasNext()) {
        i.next();
        QTreeWidgetItem * item = new QTreeWidgetItem(this);
        item->setText(0,i.key());
        item->setIcon(0,folderIcon);
        values = i.value();
        for (int j=0;j<values.count();j++) {
            if (!values[j].path().isEmpty()) {
                QTreeWidgetItem * child_item = new QTreeWidgetItem(item);
                child_item->setText(0,values[j].path());
                child_item->setIcon(0,fileIcon);
                child_item->setText(1,BytesHumanizer(values[j].size()).toString());
                child_item->setText(2,strmode(values[j].mode()));
                child_item->setText(3,values[j].date().toString("ddd MMM dd yyyy hh:mm:ss"));
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

    if (in_rect && m_pkg.isValid() && !m_pkg.isDownloaded()) emit downloadRequested(m_pkg);
    else if (m_pkg.isDownloaded()) refill();
}

