/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef PACKAGEDOWNLOADER_H
#define PACKAGEDOWNLOADER_H

#include "packageprocessor.h"

class PackageDownloader : public PackageProcessor {
    Q_OBJECT
public:
    PackageDownloader(const QList<AlpmPackage> & pkgs,ProgressView * view,QAction * cancelAction,QObject *parent = nullptr);

private slots:
    ThreadRun::RC process(const QString & pw);

private:
    QList<AlpmPackage> m_pkgs;
};

#endif // PACKAGEDOWNLOADER_H
