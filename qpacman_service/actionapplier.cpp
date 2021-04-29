/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "actionapplier.h"
#include "libalpm.h"
#include "qpacmanservice.h"

ActionApplier::ActionApplier(QPacmanService *parent) : QObject(parent) {
    m_phase = BEGIN;

    markedPackages(m_install_pkgs,m_installdeps_pkgs,m_installforced_pkgs,m_installdepsforced_pkgs,m_remove_all_pkgs,m_remove_pkgs);

    disconnect(Alpm::instance(),SIGNAL(method_finished(const QString&,const QStringList&,ThreadRun::RC)),parent,SLOT(onmethod_finished(const QString&,const QStringList&,ThreadRun::RC)));
    disconnect(Alpm::instance(),SIGNAL(method_finished(const QString&,ThreadRun::RC)),parent,SLOT(onmethod_finished(const QString&,ThreadRun::RC)));
    disconnect(Alpm::instance(),SIGNAL(method_finished(const QString&,const QList<AlpmPackage>&,ThreadRun::RC)),parent,SLOT(onmethod_finished(const QString&,const QList<AlpmPackage>&,ThreadRun::RC)));

    connect(Alpm::instance(),SIGNAL(method_finished(const QString &,ThreadRun::RC)),SLOT(processing_completed(const QString &,ThreadRun::RC)),Qt::QueuedConnection);

    if (m_remove_all_pkgs.count() > 0) {
        m_phase = REMOVE_ALL;
        Alpm::instance()->removePackages(m_remove_all_pkgs,true);
    }
    else if (m_remove_pkgs.count() > 0) {
        m_phase = REMOVE;
        Alpm::instance()->removePackages(m_remove_pkgs,false);
    }
    else if (m_install_pkgs.count() > 0) {
        m_phase = INSTALL;
        parent->install_packages(m_install_pkgs,false,m_installforced_pkgs);
    }
    else if (m_installdeps_pkgs.count() > 0) {
        m_phase = INSTALLDEPS;
        parent->install_packages(m_installdeps_pkgs,true,m_installdepsforced_pkgs);
    }
    else {
        QMetaObject::invokeMethod(parent,"error",Qt::QueuedConnection,Q_ARG(QString,tr("Nothing to process!!!")));
        m_phase = INSTALLDEPS;
        processing_completed("",ThreadRun::BAD);
    }
}

ActionApplier::~ActionApplier() {
    connect(Alpm::instance(),SIGNAL(method_finished(const QString&,const QStringList&,ThreadRun::RC)),parent(),SLOT(onmethod_finished(const QString&,const QStringList&,ThreadRun::RC)));
    connect(Alpm::instance(),SIGNAL(method_finished(const QString&,ThreadRun::RC)),parent(),SLOT(onmethod_finished(const QString&,ThreadRun::RC)));
    connect(Alpm::instance(),SIGNAL(method_finished(const QString&,const QList<AlpmPackage>&,ThreadRun::RC)),parent(),SLOT(onmethod_finished(const QString&,const QList<AlpmPackage>&,ThreadRun::RC)));
}

void ActionApplier::processing_completed(const QString & funcname,ThreadRun::RC rc) {
    switch(m_phase) {
    case REMOVE_ALL:
        remove_all_completed(funcname,rc);
        break;
    case REMOVE:
        remove_completed(funcname,rc);
        break;
    case INSTALL:
        ((QPacmanService *)parent())->remove_temp_file();
        install_completed(funcname,rc);
        break;
    case INSTALLDEPS:
        ((QPacmanService *)parent())->remove_temp_file();
        QMetaObject::invokeMethod(parent(),"method_finished",Qt::QueuedConnection,Q_ARG(QString,"ActionApplier"),Q_ARG(ThreadRun::RC,rc));
        deleteLater();
        break;
    default:
        break;
    }
}

void ActionApplier::install_completed(const QString & funcname,ThreadRun::RC ok) {
    if (ok !=ThreadRun::OK) {
        m_phase = INSTALLDEPS;
        processing_completed(funcname,ok);
        return;
    }

    if (m_installdeps_pkgs.count() > 0) {
        m_phase = INSTALLDEPS;
        ((QPacmanService *)parent())->install_packages(m_installdeps_pkgs,true,m_installdepsforced_pkgs);
    }
    else {
        m_phase = INSTALLDEPS;
        processing_completed(funcname,ok);
    }
}

void ActionApplier::remove_all_completed(const QString & funcname,ThreadRun::RC ok) {
    if (ok !=ThreadRun::OK) {
        m_phase = INSTALLDEPS;
        processing_completed(funcname,ok);
        return;
    }

    if (m_remove_pkgs.count() > 0) {
        m_phase = REMOVE;
        Alpm::instance()->removePackages(m_remove_pkgs,false);
    }
    else if (m_install_pkgs.count() > 0) {
        m_phase = INSTALL;
        ((QPacmanService *)parent())->install_packages(m_install_pkgs,false,m_installforced_pkgs);
    }
    else if (m_installdeps_pkgs.count() > 0) {
        m_phase = INSTALLDEPS;
        ((QPacmanService *)parent())->install_packages(m_installdeps_pkgs,true,m_installdepsforced_pkgs);
    }
    else {
        m_phase = INSTALLDEPS;
        processing_completed(funcname,ok);
    }
}

void ActionApplier::remove_completed(const QString & funcname,ThreadRun::RC ok) {
    if (ok !=ThreadRun::OK) {
        m_phase = INSTALLDEPS;
        processing_completed(funcname,ok);
        return;
    }

    if (m_install_pkgs.count() > 0) {
        m_phase = INSTALL;
        ((QPacmanService *)parent())->install_packages(m_install_pkgs,false,m_installforced_pkgs);
    }
    else if (m_installdeps_pkgs.count() > 0) {
        m_phase = INSTALLDEPS;
        ((QPacmanService *)parent())->install_packages(m_installdeps_pkgs,true,m_installdepsforced_pkgs);
    }
    else {
        m_phase = INSTALLDEPS;
        processing_completed(funcname,ok);
    }
}

void ActionApplier::markedPackages(QList<AlpmPackage> & install,QList<AlpmPackage> & install_asdeps,QList<AlpmPackage> & install_forced,QList<AlpmPackage> & install_asdeps_forced,QList<AlpmPackage> & removeall,QList<AlpmPackage> & remove) {
    install.clear();
    install_asdeps.clear();
    install_forced.clear();
    removeall.clear();
    remove.clear();
    for (const AlpmPackage & row: AlpmPackage::changedStatusPackages()) {
        if (row.changeStatus() == AlpmPackage::DO_INSTALL ||
            row.changeStatus() == AlpmPackage::DO_REINSTALL ||
            row.changeStatus() == AlpmPackage::DO_INSTALL_FORCE) install.append(row);
        if (row.changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS ||
            row.changeStatus() == AlpmPackage::DO_REINSTALL_ASDEPS ||
            row.changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS_FORCE) install_asdeps.append(row);
        if (row.changeStatus() == AlpmPackage::DO_INSTALL_FORCE) install_forced.append(row);
        if (row.changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS_FORCE) install_asdeps_forced.append(row);
        if (row.changeStatus() == AlpmPackage::DO_UNINSTALL_ALL) removeall.append(row);
        if (row.changeStatus() == AlpmPackage::DO_UNINSTALL) remove.append(row);
    }
}
