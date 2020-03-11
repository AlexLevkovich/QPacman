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

    ui->intervalSpin->setValue(Static::iniValue<int>("interval",DEF_OK_SEC));
    ui->err_intervalSpin->setValue(Static::iniValue<int>("err_interval",DEF_FAIL_SEC));
    ui->playCheck->setChecked(Static::iniValue<bool>("playSound",false));
}

TrayOptionsWidget::~TrayOptionsWidget() {
    delete ui;
}

void TrayOptionsWidget::okPressed() {
    Static::setIniValue("interval",ui->intervalSpin->value());
    Static::setIniValue("err_interval",ui->err_intervalSpin->value());
    Static::setIniValue("playSound",ui->playCheck->isChecked());
}

int TrayOptionsWidget::interval() const {
    return Static::iniValue<int>("interval",DEF_OK_SEC);
}

int TrayOptionsWidget::errInterval() const {
    return Static::iniValue<int>("err_interval",DEF_FAIL_SEC);
}
