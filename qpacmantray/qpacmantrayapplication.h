/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef QPACMANTRAYAPPLICATION_H
#define QPACMANTRAYAPPLICATION_H

#include "singleapplication.h"

class TrayPreferences;
class QEvent;
class QMainWindow;

class QPacmanTrayApplication : public SingleApplication {
    Q_OBJECT
public:
    QPacmanTrayApplication(int &argc, char **argv);
    ~QPacmanTrayApplication();

signals:
    void qpacmanStarted();
    void qpacmanEnded(qint64 rc);

public slots:
    void putMainWindowOnTop();

protected:
    bool notify(QObject *receiver, QEvent *event);
    void putWindowOnTop(QMainWindow * wnd);

private slots:
    void firstInstanceAttempted();
    void secondInstanceAttempted(const QStringList & args);
    void otherApplicationStarted(const QString & appname);
    void otherApplicationExited(const QString & appname,qint64 rc);

private:
    void initMainWindow();
    
    TrayPreferences * m_mainWindow;
    bool m_wasTopMost;
};

#endif // QPACMANTRAYAPPLICATION_H
