/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "removeprogressdialog.h"
#include "packagechangesdialog.h"
#include "pacmanentry.h"
#include "static.h"
#include <QMessageBox>
#include <QKeyEvent>

RemoveProgressDialog::RemoveProgressDialog(PacmanRemovePackagesReader * remover,QWidget *parent) : QProgressDialog(parent) {
    this->remover = remover;
    index = 0;

    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("Removing the packages..."));
    setMinimumDuration(500);
    setAutoReset(false);
    setLabelText(tr("Preparing..."));

    Static::makeCentered(this);

    disconnect(this,SIGNAL(canceled()),this,SLOT(cancel()));
    connect(remover,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(cancel()));
    connect(remover,SIGNAL(start_removing(const QString &)),this,SLOT(start_removing(const QString &)));
}

QStringList RemoveProgressDialog::removedPackages() const {
    return removed_packages;
}

void RemoveProgressDialog::start_removing(const QString & package) {
    setLabelText(tr("Removing %1...").arg(package));
    setValue(++index);
    removed_packages.append(package);
}

void RemoveProgressDialog::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Escape) e->ignore();
    else QProgressDialog::keyPressEvent(e);
}
