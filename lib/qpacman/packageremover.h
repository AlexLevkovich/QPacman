/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef PACKAGEREMOVER_H
#define PACKAGEREMOVER_H

#include "packageprocessor.h"

class PackageRemover : public PackageProcessor {
    Q_OBJECT
public:
    PackageRemover(const QList<AlpmPackage> & pkgs = QList<AlpmPackage>(),ProgressView * view = NULL,QAction * cancelAction = NULL,bool cascade = true,QObject *parent = nullptr);

private slots:
    ThreadRun::RC process(const QString & pw);

private:
    bool m_cascade;
    QList<AlpmPackage> m_pkgs;
};

#endif // PACKAGEINSTALLER_H
