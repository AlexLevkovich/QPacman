/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "filesdownloaddialog.h"
#include "ui_filesdownloaddialog.h"
#include <QFileInfo>
#include "byteshumanizer.h"
#include "pacmaninstallpackagesreader.h"

FilesDownloadDialog::FilesDownloadDialog(PacmanInstallPackagesReader * installer,QWidget *parent) : QDialog(parent), ui(new Ui::FilesDownloadDialog) {
    ui->setupUi(this);
    currFileNum = -1;
    m_files_count = installer->install_packages().count();

    ui->currentBar->setRange(0,100);
    ui->overalBar->setRange(0,100);
    ui->currentBar->setValue(0);
    ui->overalBar->setValue(0);

    connect(this,SIGNAL(rejected()),this,SIGNAL(canceled()));
    connect(installer,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(accept()));
    connect(installer,SIGNAL(start_download(const QString &)),this,SLOT(setNewDownload(const QString &)));
    connect(installer,SIGNAL(download_progress(int)),this,SLOT(setValue(int)));
    connect(installer,SIGNAL(contents_length_found(int)),this,SLOT(setFileLength(int)));
    connect(installer,SIGNAL(all_downloads_completed()),this,SLOT(accept()));
}

FilesDownloadDialog::~FilesDownloadDialog() {
    delete ui;
}

void FilesDownloadDialog::setNewDownload(const QString & url) {
    currFileNum++;
    ui->currentBar->setValue(0);
    ui->currentLabel->setText(QFileInfo(QUrl(url).path()).fileName());
}

void FilesDownloadDialog::setFileLength(int length) {
    ui->currentLabel->setText(ui->currentLabel->text()+" ("+BytesHumanizer((qreal)length).toString()+")");
}

void FilesDownloadDialog::setValue(int percents) {
    ui->currentBar->setValue(percents);
    ui->overalBar->setValue(((100*currFileNum)/m_files_count)+(percents/m_files_count));
}

