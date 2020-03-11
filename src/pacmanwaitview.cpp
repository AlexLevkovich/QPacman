/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanwaitview.h"
#include <QPalette>
#include <QApplication>
#include "waitindicator.h"

PacmanWaitView::PacmanWaitView(QWidget *parent) : QWidget(parent) {
    QPalette pal(palette());
    pal.setColor(QPalette::Background,QApplication::palette().color(QPalette::Base));
    setPalette(pal);

    (new WaitIndicator(this))->start();
}
