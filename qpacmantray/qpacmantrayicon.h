/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef QPACMANTRAYICON_H
#define QPACMANTRAYICON_H

#include "movietrayicon.h"
#include "themeicons.h"
#include <QAction>
#include <QMediaPlayer>

class QMenu;

class QPacmanTrayIcon : public MovieTrayIcon {
    Q_OBJECT
public:
    QPacmanTrayIcon(QAction * checkUpdatesAction,QAction * updateAction,QAction * preferencesAction,QAction * mainWindowAction,QAction * quitAction,bool * use_sound,QObject *parent = NULL);

public slots:
    void updatesFound(const QStringList & pkgs);
    void checkingInProgress();
    void updateInProgress();
    void checkingCompleted(const QString & error,int err_id);

private slots:
    void clicked();
    void fillingMenuRequest(QMenu * menu);
    void lockedFile_triggered();
    void aboutToShow();

private:
    void setIcon(ThemeIcons::Icon id);
    void setIcon(ThemeIcons::Icon id,int input_frame_height,int delay);

    QMediaPlayer good_player;
    QMediaPlayer bad_player;
    QAction * m_checkUpdatesAction;
    QAction * m_updateAction;
    QAction * m_preferencesAction;
    QAction * m_mainWindowAction;
    QAction * m_quitAction;
    QAction * m_lockFilesAction;
    bool * m_use_sound;
    ThemeIcons::Icon m_id;
    bool m_show_locking_files;
    bool m_lock_dlg_shown;
};

#endif // QPACMANTRAYICON_H
