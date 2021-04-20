/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef PACKAGEINSTALLER_H
#define PACKAGEINSTALLER_H

#include "packageprocessor.h"

class OptionalDepsDlg;

class PackageInstaller : public PackageProcessor {
    Q_OBJECT
public:
    PackageInstaller(const QList<AlpmPackage> & pkgs = QList<AlpmPackage>(),const QList<AlpmPackage> & forcedpkgs = QList<AlpmPackage>(),bool asdeps = false,ProgressView * view = NULL,QAction * cancelAction = NULL,OptionalDepsDlg * optdlg = NULL,QObject *parent = nullptr);

private slots:
    ThreadRun::RC process(const QString & pw);

private:
    QList<AlpmPackage> m_pkgs;
    QList<AlpmPackage> m_forcedpkgs;
    bool m_asdeps;
};

#endif // PACKAGEINSTALLER_H
