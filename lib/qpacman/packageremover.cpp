/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "packageremover.h"
#include "messagedialog.h"
#include "static.h"
#include <QTimer>

PackageRemover::PackageRemover(const QList<AlpmPackage> & pkgs,ProgressView * view,QAction * cancelAction,bool cascade,QObject *parent) : PackageProcessor(view,cancelAction,NULL,parent) {
    m_pkgs = pkgs;
    m_cascade = cascade;
}

ThreadRun::RC PackageRemover::process(const QString & pw) {
    return Alpm::instance()->removePackages(pw,m_pkgs,m_cascade);
}
