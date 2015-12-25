/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "combotoolbutton.h"
#include <QAction>
#include <QMenu>
#include <QKeyEvent>

ComboToolButton::ComboToolButton(QWidget *parent) : QToolButton(parent) {
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setPopupMode(QToolButton::MenuButtonPopup);
    setAutoRaise(true);

    connect(this,SIGNAL(triggered(QAction *)),this,SLOT(onMenuSelected(QAction *)));

    isFirstTime = true;
    curr_action = NULL;
}

void ComboToolButton::onMenuSelected(QAction * action) {
    setText(action->iconText());
    curr_action = action;
}

void ComboToolButton::showEvent(QShowEvent * event) {
    QToolButton::showEvent(event);

    if (isFirstTime) {
        isFirstTime = false;
        QMenu * menu = this->menu();
        if (menu != NULL) {
            QList<QAction *> actions = menu->actions();
            if (actions.count() > 0) onMenuSelected(actions.at(0));
        }
    }
}

QString ComboToolButton::iconText() const {
    return (curr_action == NULL)?text():curr_action->iconText();
}

QAction * ComboToolButton::currentAction() {
    return curr_action;
}

void ComboToolButton::keyPressEvent(QKeyEvent * event) {
    QToolButton::keyPressEvent(event);

    if (event->key() == Qt::Key_Right) showMenu();
}
