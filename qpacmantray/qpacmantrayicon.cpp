/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "qpacmantrayicon.h"
#include <QMovie>
#include <QPixmap>
#include <QFileInfo>
#include <QMediaPlayer>
#include "themeicons.h"
#include "libalpm.h"
#include <QMenu>
#include <QDebug>

class MediaPlayer : public QMediaPlayer {
private:
    MediaPlayer(const QUrl & url) : QMediaPlayer() {
        setMedia(url);
    }
    static void play(const QUrl & url) {
        MediaPlayer * player = new MediaPlayer(url);
        connect(player,&QMediaPlayer::stateChanged,[=](QMediaPlayer::State state) {
            if (state == QMediaPlayer::PlayingState) return;
            player->deleteLater();
        });
        ((QMediaPlayer *)player)->play();
    }
public:
    static void play_good() {
        play(QUrl("qrc:/sound/KDE-Sys-App-Positive.ogg"));
    }
    static void play_bad() {
        play(QUrl("qrc:/sound/KDE-Sys-App-Error.ogg"));
    }
};

QPacmanTrayIcon::QPacmanTrayIcon(bool * use_sound,QObject *parent) : MovieTrayIcon(parent) {
    m_checkUpdatesAction = new QAction(ThemeIcons::get(ThemeIcons::UPDATE_REPOS),tr("Check now"),this);
    m_checkUpdatesAction->setToolTip(tr("Downloads and updates the pacman's database"));
    m_updateAction = new QAction(ThemeIcons::get(ThemeIcons::SYNC),tr("System Update"),this);
    m_updateAction->setToolTip(tr("Starts updating of the system"));
    m_preferencesAction = new QAction(ThemeIcons::get(ThemeIcons::CONFIGURE),tr("Preferences..."),this);
    m_preferencesAction->setToolTip(tr("Shows tray icon preferences window"));
    m_quitAction = new QAction(ThemeIcons::get(ThemeIcons::QUIT),tr("Quit"),this);
    m_quitAction->setToolTip(tr("Quits from this application"));
    m_use_sound = use_sound;

    setIcon(ThemeIcons::QPACMANTRAY);

    connect(this,&QPacmanTrayIcon::clicked,this,&QPacmanTrayIcon::onclicked);
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

void QPacmanTrayIcon::onclicked() {
    if (m_checkUpdatesAction->isEnabled() && (m_id == ThemeIcons::WARNING)) m_checkUpdatesAction->trigger();
    else if (m_updateAction->isEnabled() && (m_id == ThemeIcons::QPACMANTRAY)) m_updateAction->trigger();
}

void QPacmanTrayIcon::initMenu(QMenu * menu) {
    menu->addAction(m_checkUpdatesAction);
    menu->addAction(m_updateAction);
    menu->addAction(m_preferencesAction);
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
        if (m_use_sound != NULL && *m_use_sound) MediaPlayer::play_good();
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

void QPacmanTrayIcon::checkingCompleted(const QString & error,int) {
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
        if (m_use_sound != NULL && *m_use_sound) MediaPlayer::play_bad();
    }
}
