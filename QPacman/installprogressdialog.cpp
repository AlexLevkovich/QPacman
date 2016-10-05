/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "installprogressdialog.h"
#include "packagechangesdialog.h"
#include "filesdownloaddialog.h"
#include "static.h"
#include <QMessageBox>
#include <QKeyEvent>

InstallProgressDialog::InstallProgressDialog(PacmanInstallPackagesReader * installer,uint pkgs_count,QWidget *parent) : QProgressDialog(parent) {
    this->installer = installer;
    index = 0;
    first_pkg = true;
    this->pkgs_count = pkgs_count;

    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("Installing the packages..."));
    setMinimumDuration(500);
    setAutoReset(false);
    setLabelText(tr("Preparing..."));

    disconnect(this,SIGNAL(canceled()),this,SLOT(cancel()));
    connect(installer,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(cancel()));
    connect(installer,SIGNAL(start_installing(const QString &)),this,SLOT(start_installing(const QString &)));
    connect(installer,SIGNAL(start_removing(const QString &)),this,SLOT(start_removing(const QString &)));

    setRange(0,0);
}

void InstallProgressDialog::start_installing(const QString & package) {
    if (first_pkg) {
        first_pkg = false;
        setRange(0,pkgs_count);
    }
    setLabelText(tr("Installing %1...").arg(package));
    setValue(++index);
}

void InstallProgressDialog::start_removing(const QString & package) {
    if (first_pkg) {
        first_pkg = false;
        setRange(0,pkgs_count);
    }
    setLabelText(tr("Removing %1...").arg(package));
    setValue(++index);
}

void InstallProgressDialog::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Escape) e->ignore();
    else QProgressDialog::keyPressEvent(e);
}
