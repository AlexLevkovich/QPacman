/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "removeprogressloop.h"
#include "removeprogressdialog.h"
#include "packagechangesdialog.h"
#include "pacmanentry.h"
#include "static.h"
#include <QMessageBox>
#include <QKeyEvent>

RemoveProgressLoop::RemoveProgressLoop(const QString & su_password,const QStringList & packages,bool withDeps,QWidget *parent) : QEventLoop(parent) {
    remover = NULL;
    wasCanceled = false;

    QStringList names;
    QString name;
    QString version;
    for (int i=0;i<packages.count();i++) {
        PacmanEntry::parseNameVersion(packages[i],name,version);
        names.append(name);
    }

    remover = new PacmanRemovePackagesReader(su_password,names.join(" "),withDeps,this);
    connect(remover,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(removing_packages_finished(PacmanProcessReader *)));
    connect(remover,SIGNAL(post_messages(const QString &,const QStringList &)),this,SIGNAL(post_messages(const QString &,const QStringList &)));
    connect(remover,SIGNAL(ready_to_process(double)),this,SLOT(readyToProcess(double)));
}

void RemoveProgressLoop::readyToProcess(double total_removed) {
    if (PackageChangesDialog(QStringList(),remover->packages(),0,total_removed,(QWidget *)parent()).exec() == QDialog::Rejected) {
        wasCanceled = true;
        remover->cancelRemove();
        return;
    }

    RemoveProgressDialog dlg(remover,(QWidget *)parent());
    dlg.setRange(0,(remover->packages().count() == 1)?0:remover->packages().count());
    connect(&dlg,SIGNAL(canceled()),this,SLOT(onCancel()));
    remover->beginRemove();
    dlg.exec();
    removed_packages = dlg.removedPackages();
}

void RemoveProgressLoop::onCancel() {
    if (QMessageBox::warning((QWidget *)parent(),Static::Warning_Str,Static::PacmanTerminate_Str,QMessageBox::Yes,QMessageBox::No) == QMessageBox::No) return;
    if (remover != NULL) remover->terminate();
}

void RemoveProgressLoop::removing_packages_finished(PacmanProcessReader * reader) {
    if ((reader->exitCode() != 0) || wasCanceled) {
        reader->deleteLater();
        remover = NULL;
        this->exit(QDialog::Rejected);
        return;
    }
    remover = NULL;
    this->exit(QDialog::Accepted);
}

QStringList RemoveProgressLoop::removedPackages() const {
    return removed_packages;
}
