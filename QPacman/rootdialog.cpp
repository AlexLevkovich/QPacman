/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "rootdialog.h"
#include "ui_rootdialog.h"
#include <QAbstractButton>

RootDialog::RootDialog(QWidget *parent) : QDialog(parent), ui(new Ui::RootDialog) {
    ui->setupUi(this);

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
    accept();
}

void RootDialog::on_buttonBox_rejected() {
    reject();
}

QString RootDialog::password() const {
    return ui->passEdit->text();
}
