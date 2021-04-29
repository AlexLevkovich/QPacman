/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "actionapplier.h"
#include "messagedialog.h"
#include "libalpm.h"
#include "static.h"
#include <QTimer>

ActionApplier::ActionApplier(ProgressView * view,QAction * cancelAction,OptionalDepsDlg * optdlg,QObject *parent) : PackageProcessor(view,cancelAction,optdlg,parent) {}

ThreadRun::RC ActionApplier::process(const QString & pw) {
    return Alpm::instance()->processPackages(pw);
}
