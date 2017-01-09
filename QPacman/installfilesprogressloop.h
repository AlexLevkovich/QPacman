/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef INSTALLFILESPROGRESSLOOP_H
#define INSTALLFILESPROGRESSLOOP_H

#include "installprogressloop.h"

class InstallFilesProgressLoop : public InstallProgressLoop {
    Q_OBJECT
public:
    explicit InstallFilesProgressLoop(const QString & su_password,const QStringList & packages,QWidget *parent = 0);

protected:
    int packages_count();
};

#endif // INSTALLFILESPROGRESSDIALOG_H
