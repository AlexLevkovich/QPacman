/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "slottedeventloop.h"
#include <QTimer>

SlottedEventLoop::SlottedEventLoop(QObject *parent) : QEventLoop(parent) {

}

void SlottedEventLoop::exit(int code) {
    QEventLoop::exit(code);
}

void SlottedEventLoop::reject() {
    QEventLoop::exit(1);
}

bool SlottedEventLoop::waitForSignal(QObject* obj,const char* signal,int timeout) {
    SlottedEventLoop loop;
    QObject::connect(obj,signal,&loop,SLOT(quit()));
    QObject::connect(obj,SIGNAL(destroyed()),&loop,SLOT(reject()));

    QTimer timer;
    if (timeout > 0) QObject::connect(&timer,SIGNAL(timeout()),&loop,SLOT(reject()));
    timer.start(timeout);

    return (loop.exec() == 0);
}
