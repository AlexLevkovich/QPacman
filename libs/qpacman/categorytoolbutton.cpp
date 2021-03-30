/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "categorytoolbutton.h"
#include "themeicons.h"
#include <QIcon>

CategoryToolButton::CategoryToolButton(QWidget *parent) : ComboToolButton(parent) {
    QIcon icon = ThemeIcons::get(ThemeIcons::FIND);
    menu.addAction(icon,tr("Search by name"),this,SLOT(onSearchByName()));
    menu.addAction(icon,tr("Search by provider"),this,SLOT(onSearchByProvider()));
    menu.addAction(icon,tr("Search in descriptions"),this,SLOT(onSearchByDesc()));
    menu.addAction(icon,tr("Search by file name"),this,SLOT(onSearchByFileName()));
    menu.addAction(icon,tr("Search by dependency"),this,SLOT(onSearchByDependency()));
    connect(this,SIGNAL(triggered(QAction *)),this,SLOT(onMenuItemSelected(QAction *)));

    is_sel = IS_NAME;
    setMenu(&menu);
    setIcon(icon);
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

void CategoryToolButton::onMenuItemSelected(QAction * /*action*/) {
    emit selected(getSelectedId());
}
