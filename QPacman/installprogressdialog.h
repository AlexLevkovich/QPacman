/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef INSTALLPROGRESSDIALOG_H
#define INSTALLPROGRESSDIALOG_H

#include <QProgressDialog>
#include "pacmaninstallpackagesreader.h"

class QKeyEvent;

class InstallProgressDialog : public QProgressDialog {
    Q_OBJECT
public:
    explicit InstallProgressDialog(PacmanInstallPackagesReader * installer,QWidget *parent = 0);

protected slots:
    void start_installing(const QString & package);
    void start_removing(const QString & package);

protected:
    void keyPressEvent(QKeyEvent *event);

    PacmanInstallPackagesReader * installer;

private:
    int index;
};

#endif // INSTALLPROGRESSDIALOG_H
