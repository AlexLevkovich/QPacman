/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanwaitview.h"

PacmanWaitView::PacmanWaitView(QWidget *parent) : QGraphicsView(parent) {
    angle = 0;

    m_busyIndicator.setBackgroundColor(palette().color(QPalette::Highlight));
    m_busyIndicator.setForegroundColor(palette().color(QPalette::Base));
    m_scene.addItem(dynamic_cast<QGraphicsItem*>(&m_busyIndicator));
    m_scene.setSceneRect(0,0,m_busyIndicator.width(),m_busyIndicator.height());

    setScene(&m_scene);
    setMinimumSize(m_busyIndicator.width(),m_busyIndicator.height());

    connect(&m_timer,SIGNAL(timeout()),this,SLOT(rotateSpinner()));
}

void PacmanWaitView::rotateSpinner() {
    double nTransX = m_busyIndicator.actualOuterRadius();
    m_busyIndicator.setTransform(QTransform().translate(nTransX, nTransX).
                                 rotate(angle).translate(-nTransX, -nTransX));

    angle += 10;
    if (angle == 360) angle = 0;
}

void PacmanWaitView::showEvent(QShowEvent * event) {
    QGraphicsView::showEvent(event);
    m_timer.start(50);
}

void PacmanWaitView::hideEvent(QHideEvent * event) {
    QGraphicsView::hideEvent(event);
    m_timer.stop();
}
