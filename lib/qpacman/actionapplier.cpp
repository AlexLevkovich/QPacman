/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "actionapplier.h"
#include "static.h"

ActionApplier::ActionApplier(const QList<AlpmPackage> & remove_all_pkgs,const QList<AlpmPackage> & remove_pkgs,const QList<AlpmPackage> & install_pkgs,const QList<AlpmPackage> & installdeps_pkgs,const QList<AlpmPackage> & installforced_pkgs,ProgressView * view,QAction * cancelAction,OptionalDepsDlg * optdlg,QObject *parent) : PackageProcessorBase(parent) {
    m_view = view;
    m_cancelAction = cancelAction;
    m_optionalDepsDlg = optdlg;
    m_isPartiallyOK = false;
    split_forced_pkg(installforced_pkgs);

    connect(this,&ActionApplier::completed,this,&QObject::deleteLater,Qt::QueuedConnection);
    connect(Alpm::instance(),&Alpm::pkgs_installed,this,[&]() { m_isPartiallyOK = true; });
    connect(Alpm::instance(),&Alpm::pkgs_removed,this,[&]() { m_isPartiallyOK = true; });
    if (m_optionalDepsDlg != NULL) connect(m_optionalDepsDlg,&QObject::destroyed,this,[&]() { m_optionalDepsDlg = NULL; });

    PackageProcessor * processor;
    if (remove_all_pkgs.count() > 0) {
        processor = new PackageRemover(remove_all_pkgs,view,cancelAction,true);
        connect(processor,SIGNAL(completed(ThreadRun::RC,const QString &)),SLOT(remove_all_completed(ThreadRun::RC,const QString &)),Qt::QueuedConnection);
        connect(processor,SIGNAL(logString(const QString &)),this,SIGNAL(logString(const QString &)));
        connect(processor,SIGNAL(canceled()),SIGNAL(canceled()));
        m_remove_pkgs = remove_pkgs;
        m_install_pkgs = install_pkgs;
        m_installdeps_pkgs = installdeps_pkgs;
    }
    else if (remove_pkgs.count() > 0) {
        processor = new PackageRemover(remove_pkgs,view,cancelAction,false);
        connect(processor,SIGNAL(completed(ThreadRun::RC,const QString &)),SLOT(remove_completed(ThreadRun::RC,const QString &)),Qt::QueuedConnection);
        connect(processor,SIGNAL(logString(const QString &)),this,SIGNAL(logString(const QString &)));
        connect(processor,SIGNAL(canceled()),SIGNAL(canceled()));
        m_install_pkgs = install_pkgs;
        m_installdeps_pkgs = installdeps_pkgs;
    }
    else if (install_pkgs.count() > 0) {
        processor = new PackageInstaller(install_pkgs,m_installforced_pkgs,false,view,cancelAction,m_optionalDepsDlg);
        connect(processor,SIGNAL(completed(ThreadRun::RC,const QString &)),SLOT(install_completed(ThreadRun::RC,const QString &)),Qt::QueuedConnection);
        connect(processor,SIGNAL(logString(const QString &)),this,SIGNAL(logString(const QString &)));
        connect(processor,SIGNAL(canceled()),SIGNAL(canceled()));
        m_installdeps_pkgs = installdeps_pkgs;
    }
    else if (installdeps_pkgs.count() > 0) {
        processor = new PackageInstaller(installdeps_pkgs,m_installdepsforced_pkgs,true,view,cancelAction,NULL);
        connect(processor,SIGNAL(completed(ThreadRun::RC,const QString &)),SIGNAL(completed(ThreadRun::RC,const QString &)),Qt::QueuedConnection);
        connect(processor,SIGNAL(logString(const QString &)),this,SIGNAL(logString(const QString &)));
        connect(processor,SIGNAL(canceled()),SIGNAL(canceled()));
    }
    else QMetaObject::invokeMethod(this,"completed",Qt::QueuedConnection,Q_ARG(ThreadRun::RC,ThreadRun::BAD),Q_ARG(QString,tr("Nothing to process!!!")));
}

void ActionApplier::split_forced_pkg(const QList<AlpmPackage> & installforced_pkgs) {
    m_installforced_pkgs.clear();
    m_installdepsforced_pkgs.clear();
    for (const AlpmPackage & pkg: installforced_pkgs) {
        if (pkg.changeStatus() == AlpmPackage::DO_INSTALL_FORCE) m_installforced_pkgs.append(pkg);
        else if(pkg.changeStatus() == AlpmPackage::DO_INSTALL_ASDEPS_FORCE) m_installdepsforced_pkgs.append(pkg);
    }
}

void ActionApplier::install_completed(ThreadRun::RC ok,const QString & error) {
    if (ok !=ThreadRun::OK) {
        emit completed(ok,error);
        return;
    }

    if (m_installdeps_pkgs.count() > 0) {
        PackageProcessor * processor = new PackageInstaller(m_installdeps_pkgs,m_installdepsforced_pkgs,true,m_view,m_cancelAction,m_optionalDepsDlg);
        connect(processor,SIGNAL(completed(ThreadRun::RC,const QString &)),SIGNAL(completed(ThreadRun::RC,const QString &)),Qt::QueuedConnection);
        connect(processor,SIGNAL(logString(const QString &)),this,SIGNAL(logString(const QString &)));
        connect(processor,SIGNAL(canceled()),SIGNAL(canceled()));
    }
    else emit completed(ok,QString());
}

void ActionApplier::remove_all_completed(ThreadRun::RC ok,const QString & error) {
    if (ok !=ThreadRun::OK) {
        emit completed(ok,error);
        return;
    }

    PackageProcessor * processor;
    if (m_remove_pkgs.count() > 0) {
        processor = new PackageRemover(m_remove_pkgs,m_view,m_cancelAction,false);
        connect(processor,SIGNAL(completed(ThreadRun::RC,const QString &)),SLOT(remove_completed(ThreadRun::RC,const QString &)),Qt::QueuedConnection);
        connect(processor,SIGNAL(logString(const QString &)),this,SIGNAL(logString(const QString &)));
        connect(processor,SIGNAL(canceled()),SIGNAL(canceled()));
    }
    else if (m_install_pkgs.count() > 0) {
        processor = new PackageInstaller(m_install_pkgs,m_installforced_pkgs,false,m_view,m_cancelAction,m_optionalDepsDlg);
        connect(processor,SIGNAL(completed(ThreadRun::RC,const QString &)),SLOT(install_completed(ThreadRun::RC,const QString &)),Qt::QueuedConnection);
        connect(processor,SIGNAL(logString(const QString &)),this,SIGNAL(logString(const QString &)));
        connect(processor,SIGNAL(canceled()),SIGNAL(canceled()));
    }
    else if (m_installdeps_pkgs.count() > 0) {
        processor = new PackageInstaller(m_installdeps_pkgs,m_installdepsforced_pkgs,true,m_view,m_cancelAction,NULL);
        connect(processor,SIGNAL(completed(ThreadRun::RC,const QString &)),SIGNAL(completed(ThreadRun::RC,const QString &)),Qt::QueuedConnection);
        connect(processor,SIGNAL(logString(const QString &)),this,SIGNAL(logString(const QString &)));
        connect(processor,SIGNAL(canceled()),SIGNAL(canceled()));
    }
    else emit completed(ok,QString());
}

void ActionApplier::remove_completed(ThreadRun::RC ok,const QString & error) {
    if (ok !=ThreadRun::OK) {
        emit completed(ok,error);
        return;
    }

    PackageProcessor * processor;
    if (m_install_pkgs.count() > 0) {
        processor = new PackageInstaller(m_install_pkgs,m_installforced_pkgs,false,m_view,m_cancelAction,m_optionalDepsDlg);
        connect(processor,SIGNAL(completed(ThreadRun::RC,const QString &)),SLOT(install_completed(ThreadRun::RC,const QString &)),Qt::QueuedConnection);
        connect(processor,SIGNAL(logString(const QString &)),this,SIGNAL(logString(const QString &)));
        connect(processor,SIGNAL(canceled()),SIGNAL(canceled()));
    }
    else if (m_installdeps_pkgs.count() > 0) {
        processor = new PackageInstaller(m_installdeps_pkgs,m_installdepsforced_pkgs,true,m_view,m_cancelAction,NULL);
        connect(processor,SIGNAL(completed(ThreadRun::RC)),SIGNAL(completed(ThreadRun::RC,const QString &)),Qt::QueuedConnection);
        connect(processor,SIGNAL(logString(const QString &)),this,SIGNAL(logString(const QString &)));
        connect(processor,SIGNAL(canceled()),SIGNAL(canceled()));
    }
    else emit completed(ok,QString());
}

bool ActionApplier::isPartiallyOK() const {
    return m_isPartiallyOK;
}
