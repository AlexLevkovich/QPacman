/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "combotoolbutton.h"
#include <QMenu>
#include <QKeyEvent>
#include <QChildEvent>
#include <QDebug>

ComboToolButton::ComboToolButton(QWidget *parent) : QToolButton(parent) {
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setPopupMode(QToolButton::MenuButtonPopup);
    setAutoRaise(true);

    connect(this,SIGNAL(triggered(QAction *)),this,SLOT(onMenuSelected(QAction *)));

    menuObject = NULL;
    curr_action = NULL;
}

void ComboToolButton::onMenuSelected(QAction * action) {
    setText(action->iconText());
    if (!action->icon().isNull()) setIcon(action->icon());
    curr_action = action;
}

QString ComboToolButton::iconText() const {
    QString text = (curr_action == NULL)?this->text():curr_action->iconText();
    if (text.contains("&")) text = text.replace("&","");
    return text;
}

QAction * ComboToolButton::currentAction() const {
    return curr_action;
}

void ComboToolButton::keyPressEvent(QKeyEvent * event) {
    QToolButton::keyPressEvent(event);

    if (event->key() == Qt::Key_Right) showMenu();
}

void ComboToolButton::setMenu(QMenu *menu) {
    currActionText = (curr_action != NULL)?curr_action->iconText():QString();
    QToolButton::setMenu(menu);
    menuObject = menu;
    QList<QAction *> actions = menu->actions();
    if (actions.isEmpty()) menuObject->installEventFilter(this);
    else {
        for (int i=0;i<actions.count();i++) {
            QAction * action = actions.at(i);
            if (action->menu() == NULL) {
                QMetaObject::invokeMethod(this,"onMenuSelected",Qt::QueuedConnection,Q_ARG(QAction *,action));
                QMetaObject::invokeMethod(this,"triggered",Qt::QueuedConnection,Q_ARG(QAction *,action));
                break;
            }
        }
        if (!currActionText.isEmpty()) {
            for (int i=0;i<actions.count();i++) {
                QAction * action = actions.at(i);
                if ((action->iconText() == currActionText) && (action->menu() == NULL)) {
                    QMetaObject::invokeMethod(this,"onMenuSelected",Qt::QueuedConnection,Q_ARG(QAction *,action));
                    QMetaObject::invokeMethod(this,"triggered",Qt::QueuedConnection,Q_ARG(QAction *,action));
                    currActionText.clear();
                    return;
                }
            }
        }
    }
}

bool ComboToolButton::eventFilter(QObject * obj,QEvent * event) {
    bool ret = QToolButton::eventFilter(obj,event);
    if (event->type() == QEvent::ChildAdded) {
        QChildEvent * child_event = (QChildEvent *)event;
        QAction * action = (QAction *)child_event->child();
        if (action->inherits("QAction") && (menuObject != NULL) && (action->menu() == NULL)) {
            menuObject = NULL;
            QMetaObject::invokeMethod(this,"onMenuSelected",Qt::QueuedConnection,Q_ARG(QAction *,action));
            QMetaObject::invokeMethod(this,"triggered",Qt::QueuedConnection,Q_ARG(QAction *,action));
        }
        if (action->inherits("QAction") && !currActionText.isEmpty() && (action->iconText() == currActionText) && (action->menu() == NULL)) {
            currActionText.clear();
            QMetaObject::invokeMethod(this,"onMenuSelected",Qt::QueuedConnection,Q_ARG(QAction *,action));
            QMetaObject::invokeMethod(this,"triggered",Qt::QueuedConnection,Q_ARG(QAction *,action));
        }
    }
    return ret;
}
