/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef WINDOWCENTERER_H
#define WINDOWCENTERER_H

#include <QObject>

class QEvent;
class QWidget;

class WindowCenterer : public QObject {
    Q_OBJECT
public:
    WindowCenterer(QWidget * wnd);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);
};

#endif // WINDOWCENTERER_H
