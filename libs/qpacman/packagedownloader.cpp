/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "packagedownloader.h"
#include "messagedialog.h"
#include "static.h"
#include "libalpm.h"
#include <QTimer>
#include <QDebug>

PackageDownloader::PackageDownloader(const QList<AlpmPackage *> & pkgs,ProgressView * view,QAction * cancelAction,QObject *parent) : PackageProcessor(view,cancelAction,NULL,parent) {
    m_pkgs = pkgs;
}

ThreadRun::RC PackageDownloader::process() {
    return Alpm::instance()->downloadPackages(m_pkgs);
}

