/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanwaitview.h"
#include <QPaintEvent>
#include <QPainter>
#include <QApplication>
#include <QPixmapCache>

const double PacmanWaitView::m_innerRadius = 0.8;
const double PacmanWaitView::m_outerRadius = 1.0;
const double PacmanWaitView::m_size = 150.0;

PacmanWaitView::PacmanWaitView(QWidget *parent) : QWidget(parent) {
    angle = 360;

    m_backgroundColor = QApplication::palette().color(QPalette::Highlight);
    m_foregroundColor = QApplication::palette().color(QPalette::Base);
    m_actualInnerRadius = m_size*0.5*m_innerRadius;
    m_actualOuterRadius = m_size*0.5*m_outerRadius;

    setMinimumSize(m_size,m_size);
    QPalette pal(palette());
    pal.setColor(QPalette::Background,QApplication::palette().color(QPalette::Base));
    setAutoFillBackground(true);
    setPalette(pal);

    connect(&m_timer,SIGNAL(timeout()),this,SLOT(changeAngle()),Qt::QueuedConnection);
}

void PacmanWaitView::changeAngle() {
    angle -= 10;
    if (angle == 0) angle = 360;
    update((width()-m_size)*0.5,(height()-m_size)*0.5,m_size,m_size);
}

QPixmap PacmanWaitView::generatePixmap() const {
    QString m_cacheKey = QString("%1").arg(angle);
    QPixmap pixmap;
    if (!QPixmapCache::find(m_cacheKey, &pixmap)) {
        // Set up a convenient path
        QPainterPath path;
        path.setFillRule(Qt::OddEvenFill);
        path.addEllipse(QPointF(m_actualOuterRadius,m_actualOuterRadius),m_actualOuterRadius,m_actualOuterRadius);
        path.addEllipse(QPointF(m_actualOuterRadius,m_actualOuterRadius),m_actualInnerRadius,m_actualInnerRadius);

        double nActualDiameter = 2 * m_actualOuterRadius;
        pixmap = QPixmap(nActualDiameter,nActualDiameter);
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);

        // Draw the ring background
        p.setPen(Qt::NoPen);
        p.setBrush(m_backgroundColor);
        p.setRenderHint(QPainter::Antialiasing);
        p.drawPath(path);

        // Draw the ring foreground
        QConicalGradient gradient(QPointF(m_actualOuterRadius,m_actualOuterRadius),angle);
        gradient.setColorAt(0.0,Qt::transparent);
        gradient.setColorAt(0.05,m_foregroundColor);
        gradient.setColorAt(0.8,Qt::transparent);
        p.setBrush(gradient);
        p.drawPath(path);
        p.end();

        QPixmapCache::insert(m_cacheKey,pixmap);
    }

    return pixmap;
}

void PacmanWaitView::paintEvent(QPaintEvent *event) {
    QRect pixmapRect((width()-m_size)*0.5,(height()-m_size)*0.5,m_size,m_size);
    if (!event->rect().intersects(pixmapRect)) return;

    QPainter painter(this);
    painter.drawPixmap(pixmapRect.x(),pixmapRect.y(),generatePixmap());
    painter.end();
}

void PacmanWaitView::showEvent(QShowEvent * event) {
    QWidget::showEvent(event);
    m_timer.start(50);
}

void PacmanWaitView::hideEvent(QHideEvent * event) {
    QWidget::hideEvent(event);
    m_timer.stop();
}
