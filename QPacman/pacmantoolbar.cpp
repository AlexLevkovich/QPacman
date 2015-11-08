/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmantoolbar.h"

PacmanToolBar::PacmanToolBar(QWidget *parent) : QToolBar(parent) {
}

QWidget * PacmanToolBar::findWidget(int id) {
    QList<QAction *> actions = this->actions();

    int count = 0;
    for (int i=0;i<actions.count();i++) {
        QWidget * widget = widgetForAction(actions[i]);
        if ((widget != NULL) && widget->inherits("ToolBarWidget")) {
            if (count == id) return widget;
            count++;
        }
    }

    return NULL;
}
