#include "windowcenterer.h"
#include <QWidget>
#include <QEvent>
#include <QDesktopWidget>
#include <QApplication>

WindowCenterer::WindowCenterer(QWidget * wnd) : QObject(wnd) {
    if (wnd != NULL) wnd->installEventFilter(this);
}

bool WindowCenterer::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Show) {
        QWidget * wnd = (QWidget *)obj;
        QWidget * parent = (QWidget *)wnd->parent();
        if (parent == NULL || parent->isHidden()) wnd->move(QApplication::desktop()->screen()->rect().center() - wnd->rect().center());
        else wnd->move(parent->window()->frameGeometry().topLeft() + parent->window()->rect().center() - wnd->rect().center());

    }
    return QObject::eventFilter(obj,event);
}
