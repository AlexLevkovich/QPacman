/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANWAITVIEW_H
#define PACMANWAITVIEW_H

#include <QGraphicsView>
#include "busyindicator.h"
#include <QGraphicsScene>
#include <QTimer>

class PacmanWaitView : public QGraphicsView {
    Q_OBJECT
public:
    explicit PacmanWaitView(QWidget *parent = 0);

protected:
    void showEvent(QShowEvent * event);
    void hideEvent(QHideEvent * event);

private slots:
    void rotateSpinner();

private:
    QGraphicsScene m_scene;
    BusyIndicator m_busyIndicator;
    QTimer m_timer;
    int angle;
};

#endif // PACMANWAITVIEW_H
