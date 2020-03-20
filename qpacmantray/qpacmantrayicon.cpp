/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "qpacmantrayicon.h"
#include <QMovie>
#include <QPixmap>
#include <QFileInfo>
#include "themeicons.h"
#include "libalpm.h"
#include "lockfilewaiter.h"
#include <QMenu>
#include <QDebug>

QPacmanTrayIcon::QPacmanTrayIcon(bool * use_sound,QObject *parent) : MovieTrayIcon(parent) {
    m_checkUpdatesAction = new QAction(ThemeIcons::get(ThemeIcons::UPDATE_REPOS),tr("Check now"),this);
    m_checkUpdatesAction->setToolTip(tr("Downloads and updates the pacman's database"));
    m_updateAction = new QAction(ThemeIcons::get(ThemeIcons::SYNC),tr("System Update"),this);
    m_updateAction->setToolTip(tr("Starts updating of the system"));
    m_preferencesAction = new QAction(ThemeIcons::get(ThemeIcons::CONFIGURE),tr("Preferences..."),this);
    m_preferencesAction->setToolTip(tr("Shows tray icon preferences window"));
    m_mainWindowAction = new QAction(ThemeIcons::get(ThemeIcons::QPACMAN),tr("QPacman"),this);
    m_mainWindowAction->setToolTip(tr("Loads QPacman application"));
    m_quitAction = new QAction(ThemeIcons::get(ThemeIcons::QUIT),tr("Quit"),this);
    m_quitAction->setToolTip(tr("Quits from this application"));
    m_lockFilesAction = new QAction(ThemeIcons::get(ThemeIcons::LOCKED),tr("Remove lock files"),this);
    m_use_sound = use_sound;
    m_lock_dlg_shown = false;
    m_show_locking_files = false;

    setIcon(ThemeIcons::QPACMANTRAY);

    good_player = new QMediaPlayer();
    good_player->setMedia(QUrl("qrc:/sound/KDE-Sys-App-Positive.ogg"));
    bad_player = new QMediaPlayer();
    bad_player->setMedia(QUrl("qrc:/sound/KDE-Sys-App-Error.ogg"));

    connect(this,SIGNAL(clicked()),this,SLOT(clicked()));
    connect(this,SIGNAL(menuAboutToShow(QMenu *)),this,SLOT(menuAboutToShow(QMenu *)));
    connect(m_lockFilesAction,SIGNAL(triggered()),this,SLOT(lockedFile_triggered()),Qt::QueuedConnection);
}

QPacmanTrayIcon::~QPacmanTrayIcon() {
    delete good_player;
    delete bad_player;
}

void QPacmanTrayIcon::lockedFile_triggered() {
    if (Alpm::instance() == NULL) return;

    m_lock_dlg_shown = true;
    if (LockFileWaiter(QStringList() << Alpm::instance()->lockFilePath(),QString(),true).exec() == QDialog::Accepted) {
        m_show_locking_files = false;
        m_checkUpdatesAction->trigger();
    }
    m_lock_dlg_shown = false;
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

void QPacmanTrayIcon::menuAboutToShow(QMenu *) {
    m_lockFilesAction->setVisible(m_show_locking_files && Alpm::instance() != NULL && Alpm::instance()->isLocked() && !m_lock_dlg_shown);
}

void QPacmanTrayIcon::initMenu(QMenu * menu) {
    menu->addAction(m_checkUpdatesAction);
    menu->addAction(m_updateAction);
    menu->addAction(m_preferencesAction);
    menu->addAction(m_mainWindowAction);
    menu->addAction(m_lockFilesAction);
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
        if (m_use_sound != NULL && *m_use_sound) good_player->play();
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

void QPacmanTrayIcon::checkingCompleted(const QString & error,int err_id) {
    if (error.isEmpty()) {
        QString title = tr("No new packages are available!");
        setToolTip(title,"");
        setVisible(false);
        m_show_locking_files = false;
    }
    else {
        m_show_locking_files = (err_id == Alpm::HANDLE_LOCK);
        setIcon(ThemeIcons::WARNING);
        if (!isVisible()) setVisible(true);
        QString title = tr("There were errors diring processing!");
        setToolTip(title,error);
        showMessage(title,error);
        if (m_use_sound != NULL && *m_use_sound) bad_player->play();
    }
}
