/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "treeitemsmenu.h"
#include <QActionEvent>
#include <QWidgetAction>
#include <QApplication>
#include "treemenuwidget.h"
#include <QWidgetAction>
#include <QLayout>

Q_DECLARE_METATYPE(QAction *)

TreeItemsMenu::TreeItemsMenu(QWidget *parent) : QMenu(parent) {
    isFirstTime = true;

    treeWidget = new TreeMenuWidget(this);
    treeAction = new QWidgetAction(this);
    treeAction->setDefaultWidget(treeWidget);
    connect(this,&TreeItemsMenu::aboutToShow,this,&TreeItemsMenu::onAboutToShow);
}

void TreeItemsMenu::onAboutToShow() {
    if (!isFirstTime) return;
    isFirstTime = false;

    QList<QAction *> actions = this->actions();
    for (int i=0;i<actions.count();i++) {
        if (actions[i]->isVisible()) actions[i]->setVisible(false);
    }

    addAction(treeAction);
    treeWidget->expandAll();
    treeWidget->calculateHeight();
}

void TreeItemsMenu::actionEvent(QActionEvent * e) {
    if (e->type() == QActionEvent::ActionAdded) {
        if (e->action()->inherits("QWidgetAction")) {
            if (((QWidgetAction *)e->action())->defaultWidget()->inherits("TreeMenuWidget")) QMenu::actionEvent(e);
        }
        else {
            QMenu * menu = e->action()->menu();
            if (menu != NULL) menu->installEventFilter(this);
            ((QTreeWidget *)treeWidget)->insertAction(e->before(),e->action());
        }
    }
    else if (e->type() == QActionEvent::ActionRemoved) {
        if (e->action()->inherits("QWidgetAction")) QMenu::actionEvent(e);
        else treeWidget->removeAction(e->action());
    }
    else if (e->type() == QActionEvent::ActionChanged) {
        if (e->action()->inherits("QWidgetAction")) QMenu::actionEvent(e);
        else {
            QActionEvent event(QEvent::ActionChanged,e->action(),e->before());
            QApplication::sendEvent(treeWidget,&event);
        }
    }
}

bool TreeItemsMenu::eventFilter(QObject * obj,QEvent * event) {
    QActionEvent * e = (QActionEvent *)event;
    if (event->type() == QActionEvent::ActionAdded) {
         QMenu * menu = e->action()->menu();
         if (menu != NULL) {
             menu->installEventFilter(this);
         }
         treeWidget->insertAction(((QMenu *)obj)->menuAction(),e->before(),e->action());
         return true;
    }
    else if (event->type() == QActionEvent::ActionRemoved) {
        if (e->action()->inherits("QWidgetAction")) QMenu::actionEvent(e);
        else treeWidget->removeAction(e->action());
        return true;
    }
    else if (event->type() == QActionEvent::ActionChanged) {
        if (e->action()->inherits("QWidgetAction")) QMenu::actionEvent(e);
        else {
            QActionEvent event(QEvent::ActionChanged,e->action(),e->before());
            QApplication::sendEvent(treeWidget,&event);
        }
        return true;
    }

    return QMenu::eventFilter(obj,event);
}
