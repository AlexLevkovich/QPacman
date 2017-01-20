/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmaninfobrowser.h"
#include <QDesktopServices>
#include <QApplication>
#include <QMenu>
#include <QProcess>
#include "pacmaninfobrowserdocument.h"
#include "static.h"
#include <QTextDocumentFragment>
#include <QClipboard>
#include <QKeyEvent>
#include <QDebug>

PacmanInfoBrowser::PacmanInfoBrowser(QWidget *parent) : QTextBrowser(parent) {
    QDesktopServices::setUrlHandler("mailto",this,"openUrl");

    m_model = NULL;
    setContextMenuPolicy(Qt::CustomContextMenu);
    QTextDocument * old_doc = document();
    old_doc->setParent(NULL);
    setDocument(new PacmanInfoBrowserDocument(this));
    old_doc->deleteLater();
    connect(this,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(showContextMenu(const QPoint &)));
}

void PacmanInfoBrowser::openUrl(const QUrl & url) {
    QProcess::startDetached("xdg-email" + QLatin1Char(' ') + QString::fromLatin1(url.toEncoded().constData()));
}

void PacmanInfoBrowser::setSource(const QUrl & name) {
    if (name.scheme() != "qpc") QDesktopServices::openUrl(name);
    else {
        QString type = name.host();
        QString package = name.path().mid(1);
        if (type == "package") emit packageUrlSelected(package);
        if (type == "group") emit groupUrlSelected(package);
        if (type == "reason") emit reasonUrlSelected(package);
    }
}

void PacmanInfoBrowser::showContextMenu(const QPoint &pt) {
    QMenu menu;
    menu.addAction(QIcon("://pics/edit-copy.png"),tr("Copy"),this,SLOT(copy_selected()))->setEnabled(!textCursor().selection().isEmpty());
    menu.addAction(QIcon("://pics/edit-select-all.png"),tr("Select All"),this,SLOT(selectAll()));
    menu.exec(mapToGlobal(pt));
}

void PacmanInfoBrowser::copy_selected() {
    QApplication::clipboard()->setText(Static::htmlFragmentToText(this->textCursor().selection()));
}

void PacmanInfoBrowser::setModel(PacmanItemModel * model) {
    m_model = model;
    ((PacmanInfoBrowserDocument *)document())->setModel(m_model);
}

void PacmanInfoBrowser::clearImageCache() {
    ((PacmanInfoBrowserDocument *)document())->clearImageCache();
}

void PacmanInfoBrowser::keyPressEvent(QKeyEvent *e) {
    if (e->matches(QKeySequence::Copy)) {
        copy_selected();
        e->ignore();
    }
    else QTextBrowser::keyPressEvent(e);
}
