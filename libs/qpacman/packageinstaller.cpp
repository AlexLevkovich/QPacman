/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "packageinstaller.h"
#include "progressview.h"
#include "messagedialog.h"
#include "static.h"
#include <QTimer>
#include <QDebug>

PackageInstaller::PackageInstaller(const QList<AlpmPackage *> & pkgs,const QList<AlpmPackage *> & forcedpkgs,bool asdeps,ProgressView * view,QAction * cancelAction,OptionalDepsDlg * optdlg,QObject *parent) : PackageProcessor(view,cancelAction,optdlg,parent) {
    m_pkgs = pkgs;
    m_asdeps = asdeps;
    m_forcedpkgs = forcedpkgs;
}

ThreadRun::RC PackageInstaller::process() {
    return Alpm::instance()->installPackages(m_pkgs,m_asdeps,m_forcedpkgs);
}
