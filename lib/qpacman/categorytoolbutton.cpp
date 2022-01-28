/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "categorytoolbutton.h"
#include "themeicons.h"
#include <QIcon>

CategoryToolButton::CategoryToolButton(QWidget *parent) : ComboToolButton(parent) {
    QIcon icon = ThemeIcons::get(ThemeIcons::FIND);
    name_action = menu.addAction(icon,tr("Search by name"),this,SLOT(onSearchByName()));
    provider_action = menu.addAction(icon,tr("Search by provider"),this,SLOT(onSearchByProvider()));
    desc_action = menu.addAction(icon,tr("Search in descriptions"),this,SLOT(onSearchByDesc()));
    filename_action = menu.addAction(icon,tr("Search by file name"),this,SLOT(onSearchByFileName()));
    dep_action = menu.addAction(icon,tr("Search by dependency"),this,SLOT(onSearchByDependency()));
    connect(this,&CategoryToolButton::triggered,this,&CategoryToolButton::onMenuItemSelected);

    is_sel = IS_NAME;
    setMenu(&menu);
    setIcon(icon);
}

void CategoryToolButton::setItemId(CategoryToolButton::ItemId id) {
    QAction * action = NULL;
    switch(id) {
    case IS_NAME:
        action = name_action;
        break;
    case IS_PROVIDER:
        action = provider_action;
        break;
    case IS_DEPENDENCY:
        action = dep_action;
        break;
    case IS_DESC:
        action = desc_action;
        break;
    case IS_FILE_NAME:
        action = filename_action;
        break;
    default:
        return;
    }
    action->trigger();
    QString text = action->text();
    if (text.contains("&")) text = text.replace("&","");
    setText(text);
}

void CategoryToolButton::onSearchByName() {
    is_sel = IS_NAME;
}

void CategoryToolButton::onSearchByProvider() {
    is_sel = IS_PROVIDER;
}

void CategoryToolButton::onSearchByDependency() {
    is_sel = IS_DEPENDENCY;
}

void CategoryToolButton::onSearchByDesc() {
    is_sel = IS_DESC;
}

void CategoryToolButton::onSearchByFileName() {
    is_sel = IS_FILE_NAME;
}

CategoryToolButton::ItemId CategoryToolButton::getSelectedId() {
    return is_sel;
}

void CategoryToolButton::onMenuItemSelected(QAction *) {
    emit selected(getSelectedId());
}
