/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "installprogressdialog.h"
#include "packagechangesdialog.h"
#include "filesdownloaddialog.h"
#include "static.h"
#include <QMessageBox>

InstallProgressDialog::InstallProgressDialog(const QStringList & packages,QWidget *parent) : QProgressDialog(parent) {
    init();

    installer = new PacmanInstallPackagesReader(packages.join(" "),this);
    connect(installer,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(installing_packages_finished(PacmanProcessReader *)));
    connect(installer,SIGNAL(start_installing(const QString &)),this,SLOT(start_installing(const QString &)));
    connect(installer,SIGNAL(start_removing(const QString &)),this,SLOT(start_removing(const QString &)));
    connect(installer,SIGNAL(post_messages(const QString &,const QStringList &)),this,SIGNAL(post_messages(const QString &,const QStringList &)));
    connect(installer,SIGNAL(ready_to_process(double,double)),this,SLOT(readyToProcess(double,double)));
    connect(installer,SIGNAL(some_providers_available(const QStringList &)),this,SLOT(someProvidersAvailable(const QStringList &)));
    connect(installer,SIGNAL(question_available(const QString &)),this,SLOT(questionAvailable(const QString &)));
    connect(installer,SIGNAL(start_download(const QString &)),this,SLOT(start_download(const QString &)));
    connect(installer,SIGNAL(all_downloads_completed()),this,SLOT(all_downloads_completed()));
}

InstallProgressDialog::InstallProgressDialog(QWidget *parent) : QProgressDialog(parent) {
    init();
}

void InstallProgressDialog::all_downloads_completed() {
    emit hidingFilesDownloadDlg();
    canBeShown = true;
    setVisible(true);
}

void InstallProgressDialog::onCancelDownload() {
    wasCanceled = true;
    onCancel();
}

void InstallProgressDialog::questionAvailable(const QString & question) {
    if (QMessageBox::question(this,Static::Question_Str,question,QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes) installer->sendAnswer('Y');
    else installer->sendAnswer('N');
}

void InstallProgressDialog::start_download(const QString & url) {
    disconnect(installer,SIGNAL(start_download(const QString &)),this,SLOT(start_download(const QString &)));
    if (wasCanceled) return;
    emit showingFilesDownloadDlg();
    FilesDownloadDialog download_dlg(installer,(QWidget *)parent());
    connect(&download_dlg,SIGNAL(canceled()),this,SLOT(onCancelDownload()));
    download_dlg.setNewDownload(url);
    download_dlg.exec();
}

void InstallProgressDialog::someProvidersAvailable(const QStringList & providers) {
    emit showingProvidersList();
    QString provider = Static::someProvidersAvailable(this,providers);
    if (!provider.isEmpty()) {
        installer->sendChosenProvider(provider);
    }
    else {
        wasCanceled = true;
        canBeShown = true;
        setVisible(true);
        installer->terminate();
    }
    emit hidingProvidersList();
}

void InstallProgressDialog::readyToProcess(double total_installed,double total_removed) {
    emit showingPackageListDlg();
    if (PackageChangesDialog(installer->install_packages(),installer->remove_packages(),total_installed,total_removed,this).exec() == QDialog::Rejected) {
        wasCanceled = true;
        canBeShown = true;
        setVisible(true);
        installer->cancelInstall();
        return;
    }
    emit hidingPackageListDlg();

    int total = packages_count();
    setRange(0,(total == 1)?0:total);
    installer->beginInstall();
}

int InstallProgressDialog::packages_count() {
    return installer->install_packages().count() + installer->remove_packages().count();
}

void InstallProgressDialog::init() {
    installer = NULL;
    index = 0;
    wasCanceled = false;
    canBeShown = false;

    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("Installing the packages..."));
    setMinimumDuration(500);
    setAutoReset(false);
    setLabelText(tr("Preparing..."));
    connect(this,SIGNAL(canceled()),this,SLOT(onCancel()));
}

void InstallProgressDialog::onCancel() {
    if (QMessageBox::warning(this,Static::Warning_Str,Static::PacmanTerminate_Str,QMessageBox::Yes,QMessageBox::No) == QMessageBox::No) return;
    canBeShown = true;
    setVisible(true);
    if (installer != NULL) installer->terminate();
}

void InstallProgressDialog::installing_packages_finished(PacmanProcessReader * reader) {
    if ((reader->exitCode() != 0) || wasCanceled) {
        reader->deleteLater();
        installer = NULL;
        setVisible(true);
        reject();
        return;
    }
    reader->deleteLater();
    installer = NULL;
    setVisible(true);
    accept();
}

void InstallProgressDialog::start_installing(const QString & package) {
    setLabelText(tr("Installing %1...").arg(package));
    setValue(++index);
}

void InstallProgressDialog::start_removing(const QString & package) {
    setLabelText(tr("Removing %1...").arg(package));
    setValue(++index);
}

void InstallProgressDialog::showEvent(QShowEvent * event) {
    if (!canBeShown) QMetaObject::invokeMethod(this,"_hide",Qt::QueuedConnection);
    else QProgressDialog::showEvent(event);
}

void InstallProgressDialog::_hide() {
    QWidget::setVisible(false);
}
