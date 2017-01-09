/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include "jsondbsignals.h"
#if QT_VERSION >= 0x050000
#include <QLockFile>
#else
#include "qlockfile.h"
#endif
#include <QFileSystemWatcher>
#include <QString>

class QWidget;

class SingleApplication : public QApplication {
    Q_OBJECT
public:
    explicit SingleApplication(int & argc, char ** argv);
    void setMainWidget(QWidget * mainWidget);
    bool error() const { return m_error; }

protected slots:
    void tempDirectoryChanged();
    void terminate();

private:
    QWidget * m_mainWidget;
    JsonDbSignals signalHandler;
    QLockFile sharedLock;
    QFileSystemWatcher watcher;
    bool client_loaded;
    bool m_error;

    static const QString client_lock_file;
    static const QString tray_show_file;
};

#endif // SINGLEAPPLICATION_H
