/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "lockfilewaiter.h"
#include "ui_lockfilewaiter.h"

LockFileWaiter::LockFileWaiter(const QStringList & files,const QString & infoText,QWidget *parent) :  QDialog(parent), ui(new Ui::LockFileWaiter) {
    ui->setupUi(this);

    if (!infoText.isEmpty()) ui->infoLabel->setText(QString("<html><head/><body><p><span style=\" color:#ff0000;\">%1</span></p></body></html>").arg(infoText));
    connect(&fw,SIGNAL(rejected(const QString &,Inotifier::Error)),SLOT(pathRejected(const QString &,Inotifier::Error)));

    for (int i=0;i<files.count();i++) {
        ui->filesList->appendPlainText(files.at(i));
        if (QFile(files.at(i)).exists()) fw.addPath(files.at(i));
    }
}

LockFileWaiter::~LockFileWaiter() {
    delete ui;
}

void LockFileWaiter::showEvent(QShowEvent * event) {
    QDialog::showEvent(event);
    pathRejected("",Inotifier::Invalid);
}

void LockFileWaiter::on_buttonBox_rejected() {
    reject();
}

void LockFileWaiter::pathRejected(const QString &,Inotifier::Error) {
    if (fw.count() <= 0) accept();
}

void LockFileWaiter::on_deleteButton_clicked() {
    for (QString path: fw.paths()) {
        QFile(path).remove();
    }
}
