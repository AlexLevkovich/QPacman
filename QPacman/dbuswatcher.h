/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef DBUSWATCHER_H
#define DBUSWATCHER_H

#include <QObject>
#include <QDBusServiceWatcher>

class DBusWatcher : public QObject {
    Q_OBJECT
public:
    explicit DBusWatcher(QObject *parent = 0);

signals:
    void loaded();
    void unloaded();

private slots:
    void serviceLoaded();
    void serviceUnloaded();

private:
    QDBusServiceWatcher m_system;
};

#endif // DBUSWATCHER_H
