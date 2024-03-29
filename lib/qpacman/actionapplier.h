/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ACTIONAPPLIER_H
#define ACTIONAPPLIER_H

#include "packageprocessor.h"

class OptionalDepsDlg;
class QAction;
class ProgressView;

class ActionApplier : public PackageProcessor {
    Q_OBJECT
public:
    ActionApplier(ProgressView * view = nullptr,QAction * cancelAction = nullptr,OptionalDepsDlg * optdlg = nullptr,QObject *parent = nullptr);

private slots:
    ThreadRun::RC process(const QString & pw);
};

#endif // PACKAGEINSTALLER_H
