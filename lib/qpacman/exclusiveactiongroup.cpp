/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "exclusiveactiongroup.h"

ExclusiveActionGroup::ExclusiveActionGroup(QObject *parent) : QActionGroup(parent) {
    setExclusive(false);
    connect(this,&QActionGroup::triggered,this,&ExclusiveActionGroup::ontriggered);
}

void ExclusiveActionGroup::ontriggered(QAction * action) {
    for (QAction * & act: this->actions()) {
        if (act != action) act->setChecked(false);
    }
}

QAction * ExclusiveActionGroup::checkedAction() const {
    for (QAction * & action: this->actions()) {
        if (action->isChecked()) return action;
    }
    return NULL;
}

void ExclusiveActionGroup::uncheckAllActions() {
    for (QAction * & action: this->actions()) {
        action->setChecked(false);
    }
}
