/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "removeprogressdialog.h"
#include "packagechangesdialog.h"
#include "pacmanentry.h"
#include "static.h"
#include <QMessageBox>

RemoveProgressDialog::RemoveProgressDialog(const QStringList & packages,bool withDeps,QWidget *parent) : QProgressDialog(parent) {
    remover = NULL;
    index = 0;
    wasCanceled = false;
    canBeShown = false;

    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("Removing the packages..."));
    setMinimumDuration(500);
    setAutoReset(false);
    setLabelText(tr("Preparing..."));
    connect(this,SIGNAL(canceled()),this,SLOT(onCancel()));

    QStringList names;
    QString name;
    QString version;
    for (int i=0;i<packages.count();i++) {
        PacmanEntry::parseNameVersion(packages[i],name,version);
        names.append(name);
    }

    remover = new PacmanRemovePackagesReader(names.join(" "),withDeps,this);
    connect(remover,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(removing_packages_finished(PacmanProcessReader *)));
    connect(remover,SIGNAL(start_removing(const QString &)),this,SLOT(start_removing(const QString &)));
    connect(remover,SIGNAL(post_messages(const QString &,const QStringList &)),this,SIGNAL(post_messages(const QString &,const QStringList &)));
    connect(remover,SIGNAL(ready_to_process(double)),this,SLOT(readyToProcess(double)));
}

void RemoveProgressDialog::readyToProcess(double total_removed) {
    if (PackageChangesDialog(QStringList(),remover->packages(),0,total_removed,this).exec() == QDialog::Rejected) {
        wasCanceled = true;
        canBeShown = true;
        remover->cancelRemove();
        setVisible(true);
        return;
    }

    setRange(0,(remover->packages().count() == 1)?0:remover->packages().count());
    canBeShown = true;
    remover->beginRemove();
    setVisible(true);
}

void RemoveProgressDialog::onCancel() {
    if (QMessageBox::warning(this,Static::Warning_Str,Static::PacmanTerminate_Str,QMessageBox::Yes,QMessageBox::No) == QMessageBox::No) return;
    if (remover != NULL) remover->terminate();
}

void RemoveProgressDialog::removing_packages_finished(PacmanProcessReader * reader) {
    if ((reader->exitCode() != 0) || wasCanceled) {
        reader->deleteLater();
        remover = NULL;
        reject();
        return;
    }
    remover = NULL;
    accept();
}

QStringList RemoveProgressDialog::removedPackages() const {
    return removed_packages;
}

void RemoveProgressDialog::start_removing(const QString & package) {
    setLabelText(tr("Removing %1...").arg(package));
    setValue(++index);
    removed_packages.append(package);
}

void RemoveProgressDialog::showEvent(QShowEvent * event) {
    if (!canBeShown) QMetaObject::invokeMethod(this,"_hide",Qt::QueuedConnection);
    else QProgressDialog::showEvent(event);
}

void RemoveProgressDialog::_hide() {
    QWidget::setVisible(false);
}
