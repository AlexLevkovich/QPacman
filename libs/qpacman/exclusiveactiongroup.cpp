/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "exclusiveactiongroup.h"

ExclusiveActionGroup::ExclusiveActionGroup(QObject *parent) : QActionGroup(parent) {
    setExclusive(false);
    connect(this,SIGNAL(triggered(QAction *)),SLOT(ontriggered(QAction *)));
}

void ExclusiveActionGroup::ontriggered(QAction * action) {
    QList<QAction *> actions = this->actions();
    for (int i=0;i<actions.count();i++) {
        if (actions[i] != action) actions[i]->setChecked(false);
    }
}
