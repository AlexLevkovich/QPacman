/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef QPACMANTRAYAPPLICATION_H
#define QPACMANTRAYAPPLICATION_H

#include <singleapplication.h>

class TrayPreferences;

class QPacmanTrayApplication : public SingleApplication {
    Q_OBJECT
public:
    QPacmanTrayApplication(int &argc, char *argv[]);
    ~QPacmanTrayApplication();

private slots:
    void secondary_init();
    void primary_init();
    void putMainWindowOnTop();

protected:
    bool notify(QObject *receiver, QEvent *event);

private:
    TrayPreferences * m_mainWindow;
    bool m_wasTopMost;
};

#endif // QPACMANTRAYAPPLICATION_H
