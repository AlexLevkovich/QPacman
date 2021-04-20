/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "trayoptionswidget.h"
#include "ui_trayoptionswidget.h"
#include "static.h"

#define DEF_OK_SEC   60
#define DEF_FAIL_SEC 5

TrayOptionsWidget::TrayOptionsWidget(QWidget *parent) : CategoryWidget(parent), ui(new Ui::TrayOptionsWidget) {
    ui->setupUi(this);

    ui->intervalSpin->setValue(interval());
    ui->err_intervalSpin->setValue(errInterval());
    ui->playCheck->setChecked(doPlaySound());
}

TrayOptionsWidget::~TrayOptionsWidget() {
    delete ui;
}

void TrayOptionsWidget::okPressed() {
    setIniValue("interval",ui->intervalSpin->value());
    setIniValue("err_interval",ui->err_intervalSpin->value());
    setIniValue("playSound",ui->playCheck->isChecked());
}

int TrayOptionsWidget::interval() const {
    return iniValue<int>("interval",DEF_OK_SEC);
}

int TrayOptionsWidget::errInterval() const {
    return iniValue<int>("err_interval",DEF_FAIL_SEC);
}

bool TrayOptionsWidget::doPlaySound() const {
    return iniValue<bool>("playSound",false);
}
