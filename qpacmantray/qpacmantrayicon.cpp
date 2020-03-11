/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "qpacmantrayicon.h"
#include "static.h"
#include <QMovie>
#include <QPixmap>
#include <QFileInfo>
#include "themeicons.h"
#include <QMenu>
#include <QDebug>

QPacmanTrayIcon::QPacmanTrayIcon(QAction * checkUpdatesAction,QAction * updateAction,QAction * preferencesAction,QAction * mainWindowAction,QAction * quitAction,QObject *parent) : MovieTrayIcon(parent) {
    m_checkUpdatesAction = checkUpdatesAction;
    m_updateAction = updateAction;
    m_preferencesAction = preferencesAction;
    m_mainWindowAction = mainWindowAction;
    m_quitAction = quitAction;

    setIcon(ThemeIcons::get(ThemeIcons::QPACMANTRAY));

    good_player.setMedia(QUrl("qrc:/sound/KDE-Sys-App-Positive.ogg"));
    bad_player.setMedia(QUrl("qrc:/sound/KDE-Sys-App-Error.ogg"));

    connect(this,SIGNAL(clicked()),this,SLOT(clicked()));
    connect(this,SIGNAL(fillingMenuRequest(QMenu *)),this,SLOT(fillingMenuRequest(QMenu *)));
}

void QPacmanTrayIcon::clicked() {
    if (m_checkUpdatesAction->isEnabled()) m_checkUpdatesAction->trigger();
}

void QPacmanTrayIcon::fillingMenuRequest(QMenu * menu) {
    menu->addAction(m_checkUpdatesAction);
    menu->addAction(m_updateAction);
    menu->addAction(m_preferencesAction);
    menu->addAction(m_mainWindowAction);
    menu->addSeparator();
    menu->addAction(m_quitAction);
}

void QPacmanTrayIcon::updatesFound(const QStringList & pkgs) {
    if (pkgs.count() > 0) {
        setIcon(ThemeIcons::name(ThemeIcons::QPACMANTRAY),22,1);
        if (!isVisible()) setVisible(true);
        QString message = pkgs.join('\n');
        QString title = tr("New packages are available:");
        setToolTip(title,message);
        showMessage(title,message);
        if (Static::iniValue<bool>("playSound",false)) good_player.play();
    }
    else {
        QString title = tr("No new packages are available!");
        setToolTip(title,"");
        setVisible(false);
    }
}

void QPacmanTrayIcon::checkingInProgress() {
    setIcon(ThemeIcons::name(ThemeIcons::CHECKING_MOVIE),8,300);
    if (!isVisible()) setVisible(true);
    QString title = tr("The availability of new packages are being checked...");
    setToolTip(title,"");
}

void QPacmanTrayIcon::updateInProgress() {
    setIcon(ThemeIcons::name(ThemeIcons::WAITING_MOVIE),22,300);
    if (!isVisible()) setVisible(true);
    QString title = tr("The packages are being updated...");
    setToolTip(title,"");
    showMessage(title,"");
}

void QPacmanTrayIcon::checkingCompleted(const QString & error) {
    if (error.isEmpty()) {
        QString title = tr("No new packages are available!");
        setToolTip(title,"");
        setVisible(false);
    }
    else {
        setIcon(ThemeIcons::get(ThemeIcons::WARNING));
        if (!isVisible()) setVisible(true);
        QString title = tr("There were errors diring processing!");
        setToolTip(title,error);
        showMessage(title,error);
        if (Static::iniValue<bool>("playSound",false)) bad_player.play();
    }
}
