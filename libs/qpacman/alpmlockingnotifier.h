/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ALPMLOCKINGNOTIFIER_H
#define ALPMLOCKINGNOTIFIER_H

#include <QObject>

class AlpmLockingNotifier : public QObject {
    Q_OBJECT
public:
    explicit AlpmLockingNotifier(QObject *parent = nullptr);

signals:
    void unlocked();

private slots:
    void alpm_locking_changed(bool locked);

};

#endif // ALPMLOCKINGNOTIFIER_H
