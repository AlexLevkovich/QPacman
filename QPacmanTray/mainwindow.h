/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMap>
#include <QTimer>
#include <QMovie>
#if QT_VERSION >= 0x050000
#include <QLockFile>
#else
#include "qlockfile.h"
#endif
#include "externalplayer.h"

namespace Ui {
class MainWindow;
}

class PacmanProcessReader;
class ErrorDialog;
class QPushButton;
class PacmanDBRefresher;

class MainWindow : public QMainWindow {
    Q_OBJECT

    friend class TimeOutEnd;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_quitButtonBox_accepted();
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    void onUpdate();
    void onSettings();
    void onAbout();
    void timeout();
    void checking_frame();
    void db_refreshed(PacmanProcessReader * reader);
    void executePacman();
    void updateRequested();
    void showErrorsRequested();
    void onQuit();
    void was_error(const QString & error,const QString & command);
    void onShowErrors();
    void _areUpdates(const QStringList & packages);
    void onGuiExited();
    void onGuiStarted();
    void onActionCheckUIUpdate(bool flag);
    void onActionUpdateUIUpdate(bool flag);
    void onActionErrorsUIUpdate(bool flag);

public slots:
    void terminate();

signals:
    void actionCheckUIUpdate(bool flag);
    void actionUpdateUIUpdate(bool flag);
    void actionErrorsUIUpdate(bool flag);

protected:
    void showEvent(QShowEvent * event);

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon * tray;
    QAction * checkUpdatesAction;
    QAction * updateAction;
    QAction * errorAction;
    QTimer timer;
    QMovie movie;
    QStringList packages;
    int interval;
    int err_interval;
    bool doPlaySound;
    bool isCheckingUpdates;
    bool wasError;
    bool isConnected;
    ErrorDialog * errorDialog;
    QMap<QString,QString> m_errors;
    QIcon errorIcon;
    QIcon packageIcon;
    QPushButton * quitButton;
    PacmanDBRefresher * refresher;
    QLockFile checkingLock;
    ExternalPlayer good_player;
    ExternalPlayer bad_player;

    bool isGuiAppActive();
    void playSoundForCheckOk();
    void playSoundForWasError();
    void showTray(const QIcon & icon);
    void hideTray();
};

#endif // MAINWINDOW_H
