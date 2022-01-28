/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "repotoolbutton.h"
#include "static.h"
#include "themeicons.h"
#include <QMenu>
#include <QDebug>

QString RepoToolButton::RepoAll_Str;

RepoToolButton::RepoToolButton(QWidget *parent) : ComboToolButton(parent) {
    setIcon(ThemeIcons::get(ThemeIcons::REPO));
    setToolTip(tr("Filters the packages by selected repository"));
    RepoAll_Str = tr("All");
}

void RepoToolButton::fill(const QStringList & repos) {
    QString text = this->text();

    QMenu * menu = new QMenu(this);
    connect(this,&RepoToolButton::triggered,this,&RepoToolButton::onMenuItemSelected);

    QIcon icon = ThemeIcons::get(ThemeIcons::REPO);
    menu->addAction(icon,RepoAll_Str)->setProperty("All",1);
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
    if (!isFound) setText(RepoAll_Str);
}

void RepoToolButton::onMenuItemSelected(QAction * action) {
    QString repo = action->iconText();
    if (repo.contains("&")) repo = repo.replace("&","");
    if (repo == RepoAll_Str) repo.clear();
    emit selected(repo);
}

QString RepoToolButton::iconText() const {
    QString text = ComboToolButton::iconText();
    if (text == RepoAll_Str) text.clear();
    return text;
}

void RepoToolButton::selectMenuItem(const QString & text1) {
    QString text = text1;
    if (text.isEmpty()) text = RepoAll_Str;
    QList<QAction *> actions = menu()->actions();
    for (int i=0;i<actions.count();i++) {
        QString repo = actions.at(i)->text();
        if (repo.contains("&")) repo = repo.replace("&","");

        if (repo == RepoAll_Str && actions.at(i)->property("All").isValid() && repo == text) {
            setText(RepoAll_Str);
            emit selected(RepoAll_Str);
            break;
        }
        else if (repo == text) {
            setText(text);
            emit selected(text);
            break;
        }
    }
}

void RepoToolButton::selectAllMenuItem() {
    selectMenuItem(RepoAll_Str);
}
