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
#include <QEvent>
#include <QDebug>

RootDialog::RootDialog() : UnableToCloseDialog(nullptr), ui(new Ui::RootDialog) {
    ui->setupUi(this);
    firstTime = true;

#if USER_AUTH > 0
    ui->infoLabel->setText(tr("<html><head/><body><p>You need <span style=\" font-weight:600;\">an additional privileges</span> for the operations that can modify the system. Please enter <span style=\" font-weight:600;\">your</span> password below or click %1 if you prefer not to continue.</p></body></html>"));
#else
    ui->infoLabel->setText(tr("<html><head/><body><p>You need <span style=\" font-weight:600;\">root privileges</span> for the operations that can modify the system. Please enter <span style=\" font-weight:600;\">root's</span> password below or click %1 if you prefer not to continue.</p></body></html>"));
#endif

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

bool RootDialog::event(QEvent *e) {
    bool res = UnableToCloseDialog::event(e);
    if (e->type() == QEvent::Show && firstTime) {
        firstTime = false;
        ui->infoLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        QSizePolicy policy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        policy.setHeightForWidth(true);
        policy.setHeightForWidth(ui->infoLabel->wordWrap());
        ui->infoLabel->setSizePolicy(policy);
        updateGeometry();
    }
    return res;
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

