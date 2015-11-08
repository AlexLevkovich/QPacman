/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include "jsondbsignals.h"
#include "qlockfile.h"
#include "pacmanserverinterface.h"

class QWidget;

class SingleApplication : public QApplication {
    Q_OBJECT
public:
    explicit SingleApplication(int & argc, char ** argv);
    bool isStarted();
    void setMainWidget(QWidget * mainWidget) { m_mainWidget = mainWidget; }

protected:
    bool eventFilter(QObject * obj,QEvent * event);

protected slots:
    void showMainWindow();
    void guiExited();
    void guiStarted();
    void updatesList(const QStringList & list);
    void aboutToQuit();
    void dbusLoaded();
    void dbusUnloaded();

private:
    QWidget * m_mainWidget;
    JsonDbSignals signalHandler;
    QLockFile sharedLock;
    bool m_isStarted;

    void initDbusConnections();
};

#endif // SINGLEAPPLICATION_H
