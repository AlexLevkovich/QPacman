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

InstallProgressDialog::InstallProgressDialog(PacmanInstallPackagesReader * installer,QWidget *parent) : QProgressDialog(parent) {
    this->installer = installer;
    index = 0;
    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("Installing the packages..."));
    setMinimumDuration(500);
    setAutoReset(false);
    setLabelText(tr("Preparing..."));

    disconnect(this,SIGNAL(canceled()),this,SLOT(cancel()));
    connect(installer,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(cancel()));
    connect(installer,SIGNAL(start_installing(const QString &)),this,SLOT(start_installing(const QString &)));
    connect(installer,SIGNAL(start_removing(const QString &)),this,SLOT(start_removing(const QString &)));
}

void InstallProgressDialog::start_installing(const QString & package) {
    setLabelText(tr("Installing %1...").arg(package));
    setValue(++index);
}

void InstallProgressDialog::start_removing(const QString & package) {
    setLabelText(tr("Removing %1...").arg(package));
    setValue(++index);
}

void InstallProgressDialog::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Escape) e->ignore();
    else QProgressDialog::keyPressEvent(e);
}
