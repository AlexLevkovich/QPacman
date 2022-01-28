/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "custompopuptextbrowser.h"
#include "themeicons.h"
#include <QMenu>
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>
#include <QTextDocumentFragment>
#include "textimagehandler.h"
#include "widgettextobject.h"

CustomPopupTextBrowser::CustomPopupTextBrowser(QWidget *parent) : QTextBrowser(parent) {
    new TextImageHandler(this);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,&CustomPopupTextBrowser::customContextMenuRequested,this,&CustomPopupTextBrowser::showContextMenu);
}

void CustomPopupTextBrowser::showContextMenu(const QPoint &pt) {
    QMenu menu;
    menu.addAction(ThemeIcons::get(ThemeIcons::COPY),tr("Copy"),this,SLOT(copy_selected()))->setEnabled(!textCursor().selection().isEmpty());
    menu.addAction(ThemeIcons::get(ThemeIcons::SELECT_ALL),tr("Select All"),this,SLOT(selectAll()));
    menu.exec(mapToGlobal(pt));
}

void CustomPopupTextBrowser::keyPressEvent(QKeyEvent *e) {
    if (e->matches(QKeySequence::Copy)) {
        copy_selected();
        e->ignore();
    }
    else QTextBrowser::keyPressEvent(e);
}

void CustomPopupTextBrowser::copy_selected() {
    QApplication::clipboard()->setText(WidgetTextObject::selectedText(this));
}
