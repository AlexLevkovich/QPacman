/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "fileslistwidget.h"
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
    QStringList files;
    if (!pkg->isFile()) files = pkg->files();

    QString pkg_cache_path;
    if (!pkg->isInstalled() && files.isEmpty()) {
        if (reader != NULL) reader->setTerminateFlag();
        if (!Alpm::isOpen() || !pkg->isDownloaded(&pkg_cache_path) || pkg_cache_path.isEmpty()) {
            if (reader != NULL) {
                QMetaObject::invokeMethod(this,"fill",Qt::QueuedConnection,Q_ARG(AlpmPackage *,pkg));
                return;
            }
            setRootIsDecorated(false);
            m_do_refresh_picture = true;
            update();
            setSelectionMode(QAbstractItemView::NoSelection);
            return;
        }
        else {
            if (reader != NULL) {
                QMetaObject::invokeMethod(this,"fill",Qt::QueuedConnection,Q_ARG(AlpmPackage *,pkg));
                return;
            }
            m_do_refresh_picture = false;
            update();
            setRootIsDecorated(true);
            setSelectionMode(QAbstractItemView::ExtendedSelection);
            m_files.clear();

            (wait_ind = new WaitIndicator(this->viewport()))->start();
            reader = new ArchiveFileReaderLoop(new PackageFileReader(pkg_cache_path));
            connect(reader,SIGNAL(filePath(const QString &)),this,SLOT(newFilePath(const QString &)));
            connect(reader,SIGNAL(destroyed()),SLOT(readerDestroyed()));
        }
    }
    else {
        if (reader != NULL) ThreadRun::setTerminateFlag();
        if (reader != NULL) {
            QMetaObject::invokeMethod(this,"fill",Qt::QueuedConnection,Q_ARG(AlpmPackage *,pkg));
            return;
        }
        fill(files);
    }
}

void FilesListWidget::readerDestroyed() {
    if (wait_ind != NULL) delete wait_ind;
    wait_ind = NULL;
    if (!reader->isTerminateFlagSet()) fill();
    reader = NULL;
}

void FilesListWidget::newFilePath(const QString & path) {
    QFileInfo fi(path);
    if (fi.dir().path() == ".") {
        m_files["/"].append(path);
    }
    else {
        m_files[fi.dir().path()].append(fi.fileName());
    }
}

void FilesListWidget::fill(const QStringList & files) {
    m_do_refresh_picture = false;
    update();
    setRootIsDecorated(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_files.clear();

    for (int i=0;i<files.count();i++) {

        QFileInfo fi(files[i]);
        if (fi.dir().path() == ".") {
            m_files["/"].append(files[i]);
        }
        else {
            m_files[fi.dir().path()].append(fi.fileName());
        }
    }

    fill();
}

void FilesListWidget::fill() {
    QString prev_key;
    QString key;
    QStringList values;
    QMapIterator<QString, QStringList> i(m_files);
    while (i.hasNext()) {
        i.next();
        key = i.key();
        if (!prev_key.isEmpty() && ((m_files[prev_key].count() <= 0) || ((m_files[prev_key].count() == 1) && (m_files[prev_key])[0].isEmpty())) && key.contains(prev_key)) m_files.remove(prev_key);
        prev_key = key;
    }

    i = QMapIterator<QString, QStringList>(m_files);
    while (i.hasNext()) {
        i.next();
        QTreeWidgetItem * item = new QTreeWidgetItem(this);
        item->setText(0,i.key());
        item->setIcon(0,folderIcon);
        values = i.value();
        for (int j=0;j<values.count();j++) {
            if (!values[j].isEmpty()) {
                QTreeWidgetItem * child_item = new QTreeWidgetItem(item);
                child_item->setText(0,values[j]);
                child_item->setIcon(0,fileIcon);
            }
        }
    }

    expandAll();
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
    if (rootIsDecorated()) return;

    if (in_rect && (m_pkg != NULL) && !m_pkg->isDownloaded(NULL)) emit downloadRequested(m_pkg);
    else if (m_pkg->isDownloaded(NULL)) refill();
}
