/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "movieicon.h"
#include <QSvgRenderer>
#include <QPainter>
#include <QMovie>
#include <QDebug>

int MovieIcon::defaultSpeed = 1;

MovieIcon::MovieIcon(const QList<QIcon> & icons,const QSize & output_frame_size) : QObject(NULL) {
    m_speed = defaultSpeed;
    frame_id = -1;
    setSize(output_frame_size);
    add(icons);

    connect(&m_timer,SIGNAL(timeout()),SLOT(timeout()));
}

MovieIcon::MovieIcon(QMovie * movie,const QSize & output_frame_size) {
    m_speed = defaultSpeed;
    frame_id = -1;
    setSize(output_frame_size);
    add(movie);

    connect(&m_timer,SIGNAL(timeout()),SLOT(timeout()));
}

MovieIcon::MovieIcon(const QString & iconpath,int input_frame_height,const QSize & output_frame_size) : QObject(NULL) {
    m_speed = defaultSpeed;
    frame_id = -1;
    setSize(output_frame_size);
    add(iconpath,input_frame_height);

    connect(&m_timer,SIGNAL(timeout()),SLOT(timeout()));
}

MovieIcon::MovieIcon(const QString & iconpath,const QList<QPoint> & start_poses,const QSize & size,const QSize & output_frame_size) : QObject(NULL) {
    m_speed = defaultSpeed;
    frame_id = -1;
    setSize(output_frame_size);
    add(iconpath,start_poses,size);

    connect(&m_timer,SIGNAL(timeout()),SLOT(timeout()));
}

MovieIcon::MovieIcon(const QIcon & icon,const QSize & output_frame_size) : QObject(NULL) {
    m_speed = defaultSpeed;
    frame_id = -1;
    setSize(output_frame_size);
    add(icon);

    connect(&m_timer,SIGNAL(timeout()),SLOT(timeout()));
}

bool MovieIcon::add(QMovie * movie) {
    stop();

    if (m_size.isValid() && !m_size.isNull()) movie->setScaledSize(m_size);
    for (int i=0;i<movie->frameCount();i++) {
        movie->jumpToFrame(i);
        if (i == 0) setDelay(movie->nextFrameDelay());
        add(QIcon(movie->currentPixmap()));
    }
    frame_id = 0;

    return true;
}

bool MovieIcon::add(const QList<QIcon> & icons) {
    stop();

    int i;
    for (i=0;i<icons.count();i++) {
        if (icons[i].isNull()) return false;
    }
    for (i=0;i<icons.count();i++) {
        add(icons[i]);
    }
    frame_id = 0;

    return true;
}

static bool operator<(const QSize &s1, const QSize &s2) {
    return (s1.width() < s2.width()) && (s1.height() < s2.height());
}

QSize MovieIcon::iconSize(const QString & iconpath) {
    QIcon icon;
    return iconSize(iconpath,icon);
}

QSize MovieIcon::iconSize(const QString & iconpath,QIcon & icon) {
    icon = QIcon(iconpath);
    QList<QSize> sizes = icon.availableSizes();
    QSize size;
    if (sizes.count() <= 0) {
        QSvgRenderer svg(iconpath);
        QRect rect = svg.viewBox();
        if ((rect.width() != 0) && (rect.height() != 0)) {
            size = rect.size();
        }
        else return QSize();
    }
    else {
        std::sort(sizes.begin(),sizes.end());
        size = sizes.last();
    }

    return size;
}

bool MovieIcon::update(int index) {
    if (!m_is_simple[index].isEmpty()) {
        QSize size = iconSize(m_icons[index].toString());
        if (!size.isValid()) return false;

        QPixmap whole_pixmap = QIcon(m_icons[index].toString()).pixmap(size.width()*(m_size.width()/(m_is_simple[index])[0].size().width()),size.height()*(m_size.height()/(m_is_simple[index])[0].size().height()));

        QPixmap pix;
        for (int i=0;i<m_is_simple[index].count();i++) {
            pix = whole_pixmap.copy((m_is_simple[index])[i].x()*(m_size.width()/(m_is_simple[index])[0].size().width()),(m_is_simple[index])[i].y()*(m_size.height()/(m_is_simple[index])[0].size().height()),m_size.width(),m_size.height());
            putStaticImage(pix);
            m_pixmaps.append(pix);
        }
    }
    else {
        QPixmap pix = m_icons[index].value<QIcon>().pixmap(m_size);
        putStaticImage(pix);
        m_pixmaps.append(pix);
    }

    return true;
}

bool MovieIcon::add(const QString & iconpath,const QList<QRect> & rects) {
    stop();

    QIcon icon;
    QSize size = iconSize(iconpath,icon);
    if (!size.isValid()) return false;
    if (rects.count() <= 0) return false;

    if (!m_size.isValid()) m_size = rects[0].size();

    m_icons.append(iconpath);
    m_is_simple.append(rects);
    frame_id = 0;
    return update(m_icons.count()-1);
}

bool MovieIcon::add(const QString & iconpath,const QList<QPoint> & start_poses,const QSize & size) {
    QList<QRect> rects;
    for (int i=0;i<start_poses.count();i++) {
        rects.append(QRect(start_poses[i],size));
    }
    return add(iconpath,rects);
}

bool MovieIcon::add(const QString & iconpath,int input_frame_height) {
    QSize size = iconSize(iconpath);
    QList<QPoint> start_poses;
    int count = size.height() / input_frame_height;
    int y = 0;
    for (int i=0;i<count;i++) {
        start_poses.append(QPoint(0,y));
        y += input_frame_height;
    }
    return add(iconpath,start_poses,QSize(size.width(),input_frame_height));
}

bool MovieIcon::add(const QIcon & icon) {
    stop();

    if (icon.isNull()) return false;
    if (!m_size.isValid()) {
        QList<QSize> sizes = icon.availableSizes();
        if (sizes.count() <= 0) return false;
        std::sort(sizes.begin(),sizes.end());
        m_size = sizes.last();
    }
    m_icons.append(icon);
    m_is_simple.append(QList<QRect>());
    frame_id = 0;
    return update(m_icons.count()-1);
}

void MovieIcon::setSize(const QSize & output_frame_size) {
    stop();

    m_size = output_frame_size;

    updateArrays();
}

QSize MovieIcon::size() const {
    return m_size;
}

void MovieIcon::clear() {
    stop();

    m_pixmaps.clear();
    m_icons.clear();
    m_is_simple.clear();
    frame_id = -1;
}

int MovieIcon::frameCount() const {
    return m_pixmaps.count();
}

void MovieIcon::setDelay(int value) {
    stop();

    m_speed = value;
}

int MovieIcon::delay() const {
    return m_speed;
}

QPixmap MovieIcon::frame(int frame_number) const {
    if ((frame_number < 0) || (frame_number >= frameCount())) return QPixmap();

    return m_pixmaps[frame_number];
}

bool MovieIcon::start() {
    frame_id = 0;
    m_timer.start(m_speed);
    emit started();

    return true;
}

void MovieIcon::stop() {
    m_timer.stop();
    emit finished();
}

void MovieIcon::timeout() {
    emit frameChanged(frame_id);
    if (frameCount() == 1) {
        stop();
        return;
    }
    frame_id++;
    if (frame_id >= frameCount()) frame_id = 0;
}

int MovieIcon::currentFrameNumber() const {
    return frame_id;
}

void MovieIcon::updateArrays() {
    m_pixmaps.clear();
    for (int i=0;i<m_icons.count();i++) {
        update(i);
    }
}

bool MovieIcon::putStatic(const QIcon & icon,const QRect & output_place) {
    QRect rect(QPoint(0,0),m_size);
    if (!rect.contains(output_place)) return false;

    stop();

    m_static_icon = icon;
    m_static_rect = output_place;

    updateArrays();

    return true;
}

bool MovieIcon::putStaticToCenter(const QIcon & icon,const QSize & output_size) {
    return putStatic(icon,QRect((m_size.width()-output_size.width())/2,(m_size.height()-output_size.height())/2,output_size.width(),output_size.height()));
}

void MovieIcon::putStaticImage(QPixmap & frame) {
    if (m_static_icon.isNull()) return;

    QPainter painter(&frame);
    painter.drawPixmap(m_static_rect,m_static_icon.pixmap(m_static_rect.size()));
}

void MovieIcon::removeStatic() {
    m_static_icon = QIcon();
    m_static_rect = QRect();

    updateArrays();
}
