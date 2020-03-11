/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef SLOTTEDEVENTLOOP_H
#define SLOTTEDEVENTLOOP_H

#include <QEventLoop>

class SlottedEventLoop : public QEventLoop {
    Q_OBJECT
public:
    explicit SlottedEventLoop(QObject *parent = nullptr);
    static bool waitForSignal(QObject* obj, const char* signal, int timeout = 0);

public slots:
    void exit(int code);
    void reject();
};

#endif // SLOTTEDEVENTLOOP_H
