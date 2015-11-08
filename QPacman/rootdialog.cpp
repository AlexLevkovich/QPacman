/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "rootdialog.h"
#include "ui_rootdialog.h"
#include <QAbstractButton>
#include "pacmanpasswordchecker.h"
#include "static.h"


RootDialog::RootDialog(const QByteArray & salt,QWidget *parent) : QDialog(parent), ui(new Ui::RootDialog) {
    ui->setupUi(this);

    m_passwordIsOK = false;

    m_salt = salt;

    QList<QAbstractButton *> buttons = ui->buttonBox->buttons();
    for (int i=0;i<buttons.count();i++) {
        if (ui->buttonBox->standardButton(buttons[i]) == QDialogButtonBox::Cancel) {
            ui->infoLabel->setText(ui->infoLabel->text().arg(buttons[i]->text()));
            break;
        }
    }
}

RootDialog::~RootDialog() {
    delete ui;
}

void RootDialog::on_buttonBox_accepted() {
    PacmanPasswordChecker passChecker(Static::encryptUserPassword(ui->passEdit->text().toLocal8Bit().constData(),m_salt));
    passChecker.waitToComplete();
    if (passChecker.exitCode() == 0) m_passwordIsOK = true;
    accept();
}

void RootDialog::on_buttonBox_rejected() {
    reject();
}
