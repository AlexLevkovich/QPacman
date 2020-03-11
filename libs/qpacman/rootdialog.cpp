/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "rootdialog.h"
#include "static.h"
#include "themeicons.h"
#include "ui_rootdialog.h"
#include <QAbstractButton>
#include <QFontMetrics>
#include <QDebug>

RootDialog::RootDialog() : UnableToCloseDialog(NULL), ui(new Ui::RootDialog) {
    ui->setupUi(this);

    setWindowIcon(ThemeIcons::get(ThemeIcons::ROOT_DLG));
    int len = ui->label_3->fontMetrics().horizontalAdvance(ui->label_3->text());
    ui->label->setPixmap(ThemeIcons::get(ThemeIcons::DLG_PASSWORD).pixmap(len,len));

    QList<QAbstractButton *> buttons = ui->buttonBox->buttons();
    for (int i=0;i<buttons.count();i++) {
        if (ui->buttonBox->standardButton(buttons[i]) == QDialogButtonBox::Cancel) {
            ui->infoLabel->setText(ui->infoLabel->text().arg(Static::fixButtonText(buttons[i]->text())));
            break;
        }
    }
    on_passEdit_textChanged("");

    Static::makeCentered(this);
}

RootDialog::~RootDialog() {
    delete ui;
}

void RootDialog::on_buttonBox_accepted() {
    accept();
}

void RootDialog::on_buttonBox_rejected() {
    reject_requested();
}

QString RootDialog::password() const {
    return ui->passEdit->text();
}

void RootDialog::reject_requested() {
    reject();
}

void RootDialog::on_passEdit_textChanged(const QString & text) {
    QList<QAbstractButton *> buttons = ui->buttonBox->buttons();
    for (int i=0;i<buttons.count();i++) {
        if (ui->buttonBox->standardButton(buttons[i]) == QDialogButtonBox::Ok) {
            buttons[i]->setEnabled(!text.isEmpty());
            break;
        }
    }
}
