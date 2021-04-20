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
#include "qalpmtypes.h"
#include <QProcess>

namespace Ui {
class TrayPreferences;
}

class QShowEvent;
class QAction;
class QToolBar;
class ProgressView;
class QPlainTextEdit;

class TrayPreferences : public QMainWindow {
    Q_OBJECT
public:
    TrayPreferences(int timeout = 0,QWidget *parent = nullptr);
    ~TrayPreferences();

protected:
    void showEvent(QShowEvent * event);
    void resizeEvent(QResizeEvent *event);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void onCheckForUpdatesTriggered();
    void onUpdateNowTriggered();
    void onQuitTriggered();
    void updateActions(const QString & lock_path = QString(),bool locked = false);
    void post_resize_save();
    void checker_completed(ThreadRun::RC ok,const QString & error,const QStringList & updates);
    void onInstallerCompleted(ThreadRun::RC rc);

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
    QAction * actionQuit;
    QToolBar * toolBar;

    ProgressView * progressView;
    QPlainTextEdit * logView;
    QAction * cancelAction;
    QAction * logAction;
    QMainWindow * updateWindow;
};

#endif // TRAYPREFERENCES_H
