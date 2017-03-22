/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "repotoolbutton.h"
#include "static.h"
#include <QMenu>

RepoToolButton::RepoToolButton(QWidget *parent) : ComboToolButton(parent) {
    setIcon(QIcon(":pics/repository.ico"));
    setToolTip(tr("Filters the packages by selected repository"));
}

void RepoToolButton::fill(const QStringList & repos) {
    QString text = this->text();

    QMenu * menu = new QMenu(this);
    connect(this,SIGNAL(triggered(QAction *)),this,SLOT(onMenuItemSelected(QAction *)));

    QIcon icon(":pics/repository.ico");
    menu->addAction(icon,Static::RepoAll_Str);
    for (int i=0;i<repos.count();i++) {
        if (!repos[i].isEmpty()) menu->addAction(icon,repos[i]);
    }

    QMenu * old_menu = this->menu();
    setMenu(menu);
    if (old_menu != NULL) delete old_menu;

    setText(text);
    bool isFound = false;
    QList<QAction *> actions = this->menu()->actions();
    for (int i=0;i<actions.count();i++) {
        if (actions[i]->text() == text) {
            actions[i]->trigger();
            emit triggered(actions[i]);
            isFound = true;
            break;
        }
    }
    if (!isFound) setText(Static::RepoAll_Str);
}

void RepoToolButton::onMenuItemSelected(QAction * action) {
    emit selected(action->iconText());
}

void RepoToolButton::selectMenuItem(const QString & str) {
    QList<QAction *> actions = menu()->actions();
    for (int i=0;i<actions.count();i++) {
        if (actions.at(i)->text() == str) {
            setText(str);
            emit selected(str);
            break;
        }
    }
}
