/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ACTIONAPPLIER_H
#define ACTIONAPPLIER_H

#include <QObject>
#include <QList>
#include "alpmpackage.h"
#include "alpmfuture.h"

class QPacmanService;

class ActionApplier : public QObject {
    Q_OBJECT

    enum Phase {
        BEGIN = 0,
        REMOVE_ALL = 1,
        REMOVE = 2,
        INSTALL = 3,
        INSTALLDEPS = 4
    };

public:
    ActionApplier(QPacmanService * parent);
    ~ActionApplier();

signals:
    void optional_pkgs_selected(const QStringList & pkgs);

private slots:
    void processing_completed(const QString & funcname,ThreadRun::RC rc);

private:
    void remove_all_completed(const QString & funcname,ThreadRun::RC rc);
    void remove_completed(const QString & funcname,ThreadRun::RC rc);
    void install_completed(const QString & funcname,ThreadRun::RC rc);
    void init();

    QList<AlpmPackage> m_remove_all_pkgs;
    QList<AlpmPackage> m_remove_pkgs;
    QList<AlpmPackage> m_install_pkgs;
    QList<AlpmPackage> m_installdeps_pkgs;
    QList<AlpmPackage> m_installforced_pkgs;
    QList<AlpmPackage> m_installdepsforced_pkgs;
    Phase m_phase;
};

#endif // ACTIONAPPLIER_H
