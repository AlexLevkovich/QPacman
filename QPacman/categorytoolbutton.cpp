/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "categorytoolbutton.h"
#include <QIcon>

CategoryToolButton::CategoryToolButton(QWidget *parent) : ComboToolButton(parent) {
    QIcon icon(":/pics/edit-find.png");
    menu.addAction(icon,tr("Search by name"),this,SLOT(onSearchByName()));
    menu.addAction(icon,tr("Search by provider"),this,SLOT(onSearchByProvider()));
    menu.addAction(icon,tr("Search in descriptions"),this,SLOT(onSearchByDesc()));
    menu.addAction(icon,tr("Search by file name"),this,SLOT(onSearchByFileName()));
    connect(this,SIGNAL(triggered(QAction *)),this,SLOT(onMenuItemSelected(QAction *)));

    is_sel = IS_NAME;
    setMenu(&menu);
    setIcon(QIcon(":/pics/edit-find.png"));
}

void CategoryToolButton::onSearchByName() {
    is_sel = IS_NAME;
}

void CategoryToolButton::onSearchByProvider() {
    is_sel = IS_PROVIDER;
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
