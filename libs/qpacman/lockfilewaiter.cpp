/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "lockfilewaiter.h"
#include "themeicons.h"
#include "ui_lockfilewaiter.h"
#include <sys/stat.h>
#include <QDebug>

#ifdef USE_KDE
#include <KColorScheme>
#endif

LockFileWaiter::LockFileWaiter(const QStringList & files,const QString & infoText,bool show_cancel_noquit,QWidget *parent) :  QDialog(parent), ui(new Ui::LockFileWaiter) {
    ui->setupUi(this);

    setWindowIcon(ThemeIcons::get(ThemeIcons::LOCKED));

    ui->deleteButton->setIcon(ThemeIcons::get(ThemeIcons::DELETE));
    ui->cancelButton->setIcon(ThemeIcons::get(ThemeIcons::CANCEL));
    ui->quitButton->setIcon(ThemeIcons::get(ThemeIcons::QUIT));

    ui->cancelButton->setVisible(show_cancel_noquit);
    ui->quitButton->setVisible(!show_cancel_noquit);

    if (!infoText.isEmpty()) ui->infoLabel->setText(QString("<html><head/><body><p><span style=\" color:#ff0000;\">%1</span></p></body></html>").arg(infoText));
    connect(&fw,SIGNAL(rejected(const QString &,Inotifier::Error)),SLOT(pathRejected(const QString &,Inotifier::Error)));

    QFileInfo fi;
    for (int i=0;i<files.count();i++) {
        fi.setFile(files.at(i));
        if (fi.exists()) {
            if (!fi.isReadable() && (fi.ownerId() == getuid())) {
                ::chmod(fi.filePath().toLocal8Bit().constData(),S_IRUSR);
            }
            if (fi.isReadable()) fw.addPath(files.at(i));
            ui->filesList->addItem(files.at(i));
            ui->filesList->item(ui->filesList->count()-1)->setForeground(fi.isReadable()?ui->filesList->palette().color(QPalette::WindowText):
#ifdef USE_KDE
                                                                                                                         KColorScheme(QPalette::Active).foreground(KColorScheme::NegativeText));
#else
                                                                                                                         Qt::red);
#endif
        }
    }

    on_filesList_itemSelectionChanged();
}

LockFileWaiter::~LockFileWaiter() {
    delete ui;
}

void LockFileWaiter::showEvent(QShowEvent * event) {
    QDialog::showEvent(event);
    qRegisterMetaType<Inotifier::Error>("Inotifier::Error");
    QMetaObject::invokeMethod(this,"pathRejected",Qt::QueuedConnection,Q_ARG(QString,""),Q_ARG(Inotifier::Error,Inotifier::Invalid));
}

void LockFileWaiter::pathRejected(const QString &,Inotifier::Error) {
    if (fw.count() <= 0) accept();
}

void LockFileWaiter::on_deleteButton_clicked() {
    QFile(ui->filesList->selectedItems()[0]->text()).remove();
}

void LockFileWaiter::on_cancelButton_clicked() {
    reject();
}

void LockFileWaiter::on_quitButton_clicked() {
    reject();
    QCoreApplication::exit(100);
}

void LockFileWaiter::on_filesList_itemSelectionChanged() {
    ui->deleteButton->setEnabled(ui->filesList->selectedItems().count() && (ui->filesList->selectedItems()[0]->foreground() == ui->filesList->palette().color(QPalette::WindowText)));
}
