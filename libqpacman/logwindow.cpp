/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "logwindow.h"
#include "static.h"
#include "ui_logwindow.h"

LogWindow::LogWindow(QWidget *parent) : QDialog(parent), ui(new Ui::LogWindow) {
    ui->setupUi(this);

    Static::makeCentered(this);
}

LogWindow::~LogWindow() {
    delete ui;
}

void LogWindow::addText(const QString & text) {
    ui->logEdit->appendPlainText(text);
    ui->logEdit->ensureCursorVisible();
}

void LogWindow::hideEvent(QHideEvent * /*event*/) {
    emit hidden();
}
