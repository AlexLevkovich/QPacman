/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "actionapplier.h"
#include "libalpm.h"
#include "qpacmanservice.h"

ActionApplier::ActionApplier(QPacmanService *parent) : QObject(parent) {
    m_phase = BEGIN;

    init();

    disconnect(Alpm::instance(),SIGNAL(method_finished(QString,QStringList,ThreadRun::RC)),parent,SLOT(onmethod_finished(QString,QStringList,ThreadRun::RC)));
    disconnect(Alpm::instance(),SIGNAL(method_finished(QString,ThreadRun::RC)),parent,SLOT(onmethod_finished(QString,ThreadRun::RC)));
    disconnect(Alpm::instance(),SIGNAL(method_finished(QString,QList<AlpmPackage>,ThreadRun::RC)),parent,SLOT(onmethod_finished(QString,QList<AlpmPackage>,ThreadRun::RC)));
    connect(Alpm::instance(),SIGNAL(method_finished(QString,ThreadRun::RC)),SLOT(processing_completed(QString,ThreadRun::RC)),Qt::QueuedConnection);

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
    connect(Alpm::instance(),SIGNAL(method_finished(QString,QStringList,ThreadRun::RC)),parent(),SLOT(onmethod_finished(QString,QStringList,ThreadRun::RC)));
    connect(Alpm::instance(),SIGNAL(method_finished(QString,ThreadRun::RC)),parent(),SLOT(onmethod_finished(QString,ThreadRun::RC)));
    connect(Alpm::instance(),SIGNAL(method_finished(QString,QList<AlpmPackage>,ThreadRun::RC)),parent(),SLOT(onmethod_finished(QString,QList<AlpmPackage>,ThreadRun::RC)));
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
    if (ok == ThreadRun::OK) {
        if (m_installdeps_pkgs.count() > 0) {
            m_phase = INSTALLDEPS;
            ((QPacmanService *)parent())->install_packages(m_installdeps_pkgs,true,m_installdepsforced_pkgs);
            return;
        }
    }
    m_phase = INSTALLDEPS;
    processing_completed(funcname,ok);
}

void ActionApplier::remove_all_completed(const QString & funcname,ThreadRun::RC ok) {
    if (ok == ThreadRun::OK) {
        if (m_remove_pkgs.count() > 0) {
            m_phase = REMOVE;
            Alpm::instance()->removePackages(m_remove_pkgs,false);
            return;
        }
        else if (m_install_pkgs.count() > 0) {
            m_phase = INSTALL;
            ((QPacmanService *)parent())->install_packages(m_install_pkgs,false,m_installforced_pkgs);
            return;
        }
        else if (m_installdeps_pkgs.count() > 0) {
            m_phase = INSTALLDEPS;
            ((QPacmanService *)parent())->install_packages(m_installdeps_pkgs,true,m_installdepsforced_pkgs);
            return;
        }
    }
    m_phase = INSTALLDEPS;
    processing_completed(funcname,ok);
}

void ActionApplier::remove_completed(const QString & funcname,ThreadRun::RC ok) {
    if (ok == ThreadRun::OK) {
        if (m_install_pkgs.count() > 0) {
            m_phase = INSTALL;
            ((QPacmanService *)parent())->install_packages(m_install_pkgs,false,m_installforced_pkgs);
            return;
        }
        else if (m_installdeps_pkgs.count() > 0) {
            m_phase = INSTALLDEPS;
            ((QPacmanService *)parent())->install_packages(m_installdeps_pkgs,true,m_installdepsforced_pkgs);
            return;
        }
    }
    m_phase = INSTALLDEPS;
    processing_completed(funcname,ok);
}

void ActionApplier::init() {
    for (const AlpmPackage & row: AlpmPackage::changedStatusPackages()) {
        if (row.changeStatus() == AlpmPackage::DO_INSTALL ||
            row.changeStatus() == AlpmPackage::DO_REINSTALL ||
            row.changeStatus() == AlpmPackage::DO_INSTALL_FORCE) m_install_pkgs.append(row);
        if (row.changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS ||
            row.changeStatus() == AlpmPackage::DO_REINSTALL_ASDEPS ||
            row.changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS_FORCE) m_installdeps_pkgs.append(row);
        if (row.changeStatus() == AlpmPackage::DO_INSTALL_FORCE) m_installforced_pkgs.append(row);
        if (row.changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS_FORCE) m_installdepsforced_pkgs.append(row);
        if (row.changeStatus() == AlpmPackage::DO_UNINSTALL_ALL) m_remove_all_pkgs.append(row);
        if (row.changeStatus() == AlpmPackage::DO_UNINSTALL) m_remove_pkgs.append(row);
    }
}
