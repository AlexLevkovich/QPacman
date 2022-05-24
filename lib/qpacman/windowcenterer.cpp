/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "windowcenterer.h"
#include <QWidget>
#include <QEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QWindow>
#include <QScreen>

WindowCenterer::WindowCenterer(QWidget * wnd) : QObject(wnd) {
    if (wnd != nullptr) wnd->installEventFilter(this);
}

bool WindowCenterer::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Show) {
        QWidget * wnd = (QWidget *)obj;
        QWidget * parent = (QWidget *)wnd->parent();
        if (parent == nullptr || parent->isHidden()) wnd->move(QApplication::desktop()->window()->windowHandle()->screen()->geometry().center() - wnd->rect().center());
        else wnd->move(parent->window()->frameGeometry().topLeft() + parent->window()->rect().center() - wnd->rect().center());

    }
    return QObject::eventFilter(obj,event);
}
