/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "qpacmantrayicon.h"
#include <QMovie>
#include <QPixmap>
#include <QFileInfo>
#include "themeicons.h"
#include <QMenu>
#include <QDebug>

QPacmanTrayIcon::QPacmanTrayIcon(QAction * checkUpdatesAction,QAction * updateAction,QAction * preferencesAction,QAction * mainWindowAction,QAction * quitAction,bool * use_sound,QObject *parent) : MovieTrayIcon(parent) {
    m_checkUpdatesAction = checkUpdatesAction;
    m_updateAction = updateAction;
    m_preferencesAction = preferencesAction;
    m_mainWindowAction = mainWindowAction;
    m_quitAction = quitAction;
    m_use_sound = use_sound;

    setIcon(ThemeIcons::QPACMANTRAY);

    good_player.setMedia(QUrl("qrc:/sound/KDE-Sys-App-Positive.ogg"));
    bad_player.setMedia(QUrl("qrc:/sound/KDE-Sys-App-Error.ogg"));

    connect(this,SIGNAL(clicked()),this,SLOT(clicked()));
    connect(this,SIGNAL(fillingMenuRequest(QMenu *)),this,SLOT(fillingMenuRequest(QMenu *)));
}

void QPacmanTrayIcon::setIcon(ThemeIcons::Icon id) {
    const QIcon icon = ThemeIcons::get(id);
    m_id = icon.isNull()?ThemeIcons::null:id;
    MovieTrayIcon::setIcon(icon);
}

void QPacmanTrayIcon::setIcon(ThemeIcons::Icon id,int input_frame_height,int delay) {
    const QString name = ThemeIcons::name(id);
    m_id = name.isEmpty()?ThemeIcons::null:id;
    MovieTrayIcon::setIcon(name,input_frame_height,delay);
}

void QPacmanTrayIcon::clicked() {
    if (m_checkUpdatesAction->isEnabled() && (m_id == ThemeIcons::WARNING)) m_checkUpdatesAction->trigger();
    else if (m_updateAction->isEnabled() && (m_id == ThemeIcons::QPACMANTRAY)) m_updateAction->trigger();
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
        setIcon(ThemeIcons::QPACMANTRAY,22,1);
        if (!isVisible()) setVisible(true);
        QString message = pkgs.join('\n');
        QString title = tr("New packages are available:");
        setToolTip(title,message);
        showMessage(title,message);
        if (*m_use_sound) good_player.play();
    }
    else {
        QString title = tr("No new packages are available!");
        setToolTip(title,"");
        setVisible(false);
    }
}

void QPacmanTrayIcon::checkingInProgress() {
    setIcon(ThemeIcons::CHECKING_MOVIE,8,300);
    if (!isVisible()) setVisible(true);
    QString title = tr("The availability of new packages are being checked...");
    setToolTip(title,"");
}

void QPacmanTrayIcon::updateInProgress() {
    setIcon(ThemeIcons::WAITING_MOVIE,22,300);
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
        setIcon(ThemeIcons::WARNING);
        if (!isVisible()) setVisible(true);
        QString title = tr("There were errors diring processing!");
        setToolTip(title,error);
        showMessage(title,error);
        if (*m_use_sound) bad_player.play();
    }
}
