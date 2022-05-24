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

PackageDownloader::PackageDownloader(const QList<AlpmPackage> & pkgs,ProgressView * view,QAction * cancelAction,QObject *parent) : PackageProcessor(view,cancelAction,nullptr,parent) {
    m_pkgs = pkgs;
}

ThreadRun::RC PackageDownloader::process(const QString &) {
    return Alpm::instance()->downloadPackages(m_pkgs);
}

