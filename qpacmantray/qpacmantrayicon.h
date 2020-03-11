/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef QPACMANTRAYICON_H
#define QPACMANTRAYICON_H

#include "movietrayicon.h"
#include <QAction>
#include <QMediaPlayer>

class QMenu;

class QPacmanTrayIcon : public MovieTrayIcon {
    Q_OBJECT
public:
    QPacmanTrayIcon(QAction * checkUpdatesAction,QAction * updateAction,QAction * preferencesAction,QAction * mainWindowAction,QAction * quitAction,QObject *parent = NULL);

public slots:
    void updatesFound(const QStringList & pkgs);
    void checkingInProgress();
    void updateInProgress();
    void checkingCompleted(const QString & error);

private slots:
    void clicked();
    void fillingMenuRequest(QMenu * menu);

private:
    QMediaPlayer good_player;
    QMediaPlayer bad_player;
    QAction * m_checkUpdatesAction;
    QAction * m_updateAction;
    QAction * m_preferencesAction;
    QAction * m_mainWindowAction;
    QAction * m_quitAction;
};

#endif // QPACMANTRAYICON_H
