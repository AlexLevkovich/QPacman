/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "filesdownloaddialog.h"
#include "ui_filesdownloaddialog.h"
#include <QFileInfo>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QPushButton>
#include "byteshumanizer.h"
#include "static.h"
#include "pacmaninstallpackagesreader.h"
#include <QDebug>

FilesDownloadDialog::FilesDownloadDialog(PacmanUpdatePackagesReader * installer,QWidget *parent) : QDialog(parent), ui(new Ui::FilesDownloadDialog) {
    ui->setupUi(this);
    currFileNum = -1;
    doClose = false;
    m_files_count = installer->install_packages().count();

    ui->currentBar->setRange(0,100);
    ui->overalBar->setRange(0,100);
    ui->currentBar->setValue(0);
    ui->overalBar->setValue(0);

    Static::makeCentered(this);
    
#if QT_VERSION < 0x050000
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel),SIGNAL(clicked()),this,SLOT(reject()));
#endif      

    connect(installer,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(finished(PacmanProcessReader *)));
    connect(installer,SIGNAL(start_download(const QString &)),this,SLOT(setNewDownload(const QString &)));
    connect(installer,SIGNAL(download_progress(int)),this,SLOT(setValue(int)));
    connect(installer,SIGNAL(contents_length_found(int)),this,SLOT(setFileLength(int)));
    connect(installer,SIGNAL(all_downloads_completed()),this,SLOT(accept()));
}

FilesDownloadDialog::~FilesDownloadDialog() {
    delete ui;
}

void FilesDownloadDialog::setNewDownload(const QString & url) {
    QString fileName = QFileInfo(QUrl(url).path()).fileName();
    if (fileName == ui->currentLabel->text()) return;

    currFileNum++;
    ui->currentBar->setValue(0);
    ui->currentLabel->setText(fileName);
}

void FilesDownloadDialog::setFileLength(int length) {
    ui->currentLabel->setText(ui->currentLabel->text()+" ("+BytesHumanizer((double)length).toString()+")");
}

void FilesDownloadDialog::setValue(int percents) {
    ui->currentBar->setValue(percents);
    ui->overalBar->setValue(((100*currFileNum)/m_files_count)+(percents/m_files_count));
}

void FilesDownloadDialog::closeEvent(QCloseEvent *e) {
    if (doClose) e->accept();
    else {
        e->ignore();
        emit canceled();
    }
}

void FilesDownloadDialog::finished(PacmanProcessReader * reader) {
    doClose = true;
    if (reader->exitCode() == 0) accept();
    else reject();
}

void FilesDownloadDialog::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Escape) e->ignore();
    else QDialog::keyPressEvent(e);
}
