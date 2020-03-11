/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef QPACMANAPPLICATION_H
#define QPACMANAPPLICATION_H

#include "singleapplication.h"
#include "alpmpackage.h"
#include "packageprocessor.h"

class MainWindow;
class QMainWindow;
class LocalPackageMainWindow;
class QEvent;
class DBRefresher;

class QPacmanApplication : public SingleApplication {
    Q_OBJECT
public:
    QPacmanApplication(int &argc, char **argv);
    ~QPacmanApplication();
    QMainWindow * findMainWindow();

protected:
    bool notify(QObject *receiver, QEvent *event);
    void putWindowOnTop(QMainWindow * wnd);

private slots:
    void firstInstanceAttempted();
    void secondInstanceAttempted(const QStringList & args);
    void mainWindowDestroyed();
    void localMainWindowDestroyed();
    void updateWindowDestroyed();
    void dbRefreshCompleted(ThreadRun::RC rc);
    void upgradeCompleted(ThreadRun::RC rc);
    void initMainWindow();

private:
    void upgradePackages();

    MainWindow * m_mainWindow;
    LocalPackageMainWindow * m_localPkgWindow;
    bool m_wasTopMost;
    ProgressView * progressView;
    QAction * cancelAction;
    QAction * logAction;
    QMainWindow * updateWindow;
    QPlainTextEdit * logView;
};

#endif // QPACMANAPPLICATION_H
