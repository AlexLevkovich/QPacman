/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "rootdialog.h"
#include "themeicons.h"
#include "ui_rootdialog.h"
#include "windowcenterer.h"
#include "static.h"
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
            ui->infoLabel->setText(ui->infoLabel->text().arg(fixButtonText(buttons[i]->text())));
            break;
        }
    }

    new WindowCenterer(this);
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
    QString ret = ui->passEdit->text();
    return ret.isNull()?QString(""):ret;
}

void RootDialog::reject_requested() {
    reject();
}

