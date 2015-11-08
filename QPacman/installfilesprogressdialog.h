/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef INSTALLFILESPROGRESSDIALOG_H
#define INSTALLFILESPROGRESSDIALOG_H

#include "installprogressdialog.h"

class InstallFilesProgressDialog : public InstallProgressDialog {
    Q_OBJECT
public:
    explicit InstallFilesProgressDialog(const QStringList & packages,QWidget *parent = 0);

protected:
    int packages_count();
};

#endif // INSTALLFILESPROGRESSDIALOG_H
