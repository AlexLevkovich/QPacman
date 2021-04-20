/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmantoolbar.h"
#include "searchwidget.h"

PacmanToolBar::PacmanToolBar(QWidget *parent) : QToolBar(parent) {
}

SearchWidget * PacmanToolBar::findSearchWidget() {
    QList<QAction *> actions = this->actions();

    QWidget * widget;
    for (int i=0;i<actions.count();i++) {
        widget = widgetForAction(actions[i]);
        if ((widget != NULL) && widget->inherits("SearchWidget")) return (SearchWidget *)widget;
    }

    return NULL;
}
