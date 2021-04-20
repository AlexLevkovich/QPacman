/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ACTIONAPPLIER_H
#define ACTIONAPPLIER_H

#include "packageinstaller.h"
#include "packageremover.h"

class ActionApplier : public PackageProcessorBase {
    Q_OBJECT
public:
    ActionApplier(const QList<AlpmPackage> & remove_all_pkgs,const QList<AlpmPackage> & remove_pkgs,const QList<AlpmPackage> & install_pkgs,const QList<AlpmPackage> & installdeps_pkgs,const QList<AlpmPackage> & installforced_pkgs,ProgressView * view,QAction * cancelAction,OptionalDepsDlg * optdlg,QObject *parent = NULL);
    bool isPartiallyOK() const;

signals:
    void optional_pkgs_selected(const QStringList & pkgs);

private slots:
    void remove_all_completed(ThreadRun::RC ok,const QString & error);
    void remove_completed(ThreadRun::RC ok,const QString & error);
    void install_completed(ThreadRun::RC ok,const QString & error);

private:
    void split_forced_pkg(const QList<AlpmPackage> & installforced_pkgs);

    QList<AlpmPackage> m_remove_pkgs;
    QList<AlpmPackage> m_install_pkgs;
    QList<AlpmPackage> m_installdeps_pkgs;
    QList<AlpmPackage> m_installforced_pkgs;
    QList<AlpmPackage> m_installdepsforced_pkgs;
    ProgressView * m_view;
    QAction * m_cancelAction;
    OptionalDepsDlg * m_optionalDepsDlg;
    bool m_isPartiallyOK;
};

#endif // ACTIONAPPLIER_H
