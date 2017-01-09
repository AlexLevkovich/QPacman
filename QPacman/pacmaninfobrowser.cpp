/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmaninfobrowser.h"
#include <QDesktopServices>
#include <QMenu>
#include <QProcess>
#include "pacmaninfobrowserdocument.h"

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
        QString package = name.host();
        if (package == "pack") package = name.path().mid(1);
        QStringList parts = package.split(".");
        if ((parts.count() >= 2) && (parts[0] == "group")) {
            package="";
            for (int i=1;i<parts.count();i++) package+=parts[i]+".";
            emit groupUrlSelected(package.left(package.length()-1));
        }
        else if ((parts.count() >= 2) && (parts[0] == "reason")) {
            package="";
            for (int i=1;i<parts.count();i++) package+=parts[i]+".";
            emit reasonUrlSelected(package.left(package.length()-1));
        }
        else emit packageUrlSelected(package);
    }
}

void PacmanInfoBrowser::showContextMenu(const QPoint &pt) {
    QMenu * menu = createStandardContextMenu();
    QList<QAction *> actions = menu->actions();
    if (actions.count() > 2) {
        menu->removeAction(actions[1]);
    }
    menu->exec(mapToGlobal(pt));
    delete menu;
}

void PacmanInfoBrowser::setModel(PacmanItemModel * model) {
    m_model = model;
    ((PacmanInfoBrowserDocument *)document())->setModel(m_model);
}

void PacmanInfoBrowser::clearImageCache() {
    ((PacmanInfoBrowserDocument *)document())->clearImageCache();
}
