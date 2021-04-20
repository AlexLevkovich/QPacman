/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef QPACMANTRAYICON_H
#define QPACMANTRAYICON_H

#include "movietrayicon.h"
#include "themeicons.h"
#include <QAction>

class QMenu;

class QPacmanTrayIcon : public MovieTrayIcon {
    Q_OBJECT
public:
    QPacmanTrayIcon(bool * use_sound = NULL,QObject *parent = NULL);

    QAction * checkUpdatesAction() { return m_checkUpdatesAction; }
    QAction * updateAction() { return m_updateAction; }
    QAction * preferencesAction() { return m_preferencesAction; }
    QAction * quitAction() { return m_quitAction; }

    void setIcon(ThemeIcons::Icon id);
    void setIcon(ThemeIcons::Icon id,int input_frame_height,int delay);

public slots:
    void updatesFound(const QStringList & pkgs);
    void checkingInProgress();
    void updateInProgress();
    void checkingCompleted(const QString & error,int err_id);

private slots:
    void clicked();

private:
    void initMenu(QMenu * menu);

    QAction * m_checkUpdatesAction;
    QAction * m_updateAction;
    QAction * m_preferencesAction;
    QAction * m_quitAction;
    bool * m_use_sound;
    ThemeIcons::Icon m_id;
};

#endif // QPACMANTRAYICON_H
