/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef TRAYPREFERENCES_H
#define TRAYPREFERENCES_H

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QTimer>
#include "qpacmantrayicon.h"
#include <QProcess>

namespace Ui {
class TrayPreferences;
}

class QShowEvent;
class QAction;
class QToolBar;

class TrayPreferences : public QMainWindow {
    Q_OBJECT
public:
    TrayPreferences(QWidget *parent = nullptr);
    ~TrayPreferences();

protected:
    void showEvent(QShowEvent * event);
    void resizeEvent(QResizeEvent *event);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_actionCheck_for_updates_triggered();
    void on_actionUpdate_now_triggered();
    void on_actionQuit_triggered();
    void on_actionLoad_QPacman_triggered();
    void checker_ok(const QStringList & packages);
    void checker_error(const QString & error,int err_id);
    void pacman_finished(int code);
    void updateActions(const QString & lock_path = QString(),bool locked = false);
    void qpacmanStarted(const QStringList & parms);
    void qpacmanEnded(const QStringList & parms,qint64 rc);
    void post_resize_save();

signals:
    void showRequest();

private:
    Ui::TrayPreferences *ui;
    QPacmanTrayIcon * m_tray;
    bool m_blocking_operation;
    QTimer m_timer;
    bool m_use_sound;

    QAction * actionCheck_for_updates;
    QAction * actionUpdate_now;
    QAction * actionPreferences;
    QAction * actionLoad_QPacman;
    QAction * actionQuit;
    QToolBar * toolBar;
};

#endif // TRAYPREFERENCES_H
