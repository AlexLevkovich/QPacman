/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "toolbarwindow.h"
#include "ui_toolbarwindow.h"
#include <QShowEvent>

ToolbarWindow::ToolbarWindow(QWidget *parent) :  QMainWindow(parent), ui(new Ui::ToolbarWindow) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::Window);

    connect(parent->parent(),SIGNAL(actionCheckUIUpdate(bool)),this,SLOT(actionCheckUIUpdate(bool)));
    connect(parent->parent(),SIGNAL(actionUpdateUIUpdate(bool)),this,SLOT(actionUpdateUIUpdate(bool)));
    connect(parent->parent(),SIGNAL(actionErrorsUIUpdate(bool)),this,SLOT(actionErrorsUIUpdate(bool)));

    connect(ui->actionCheck_for_updates,SIGNAL(triggered()),this,SIGNAL(actionCheck_triggered()));
    connect(ui->actionUpdate_now,SIGNAL(triggered()),this,SIGNAL(actionUpdate_triggered()));
    connect(ui->actionErrors,SIGNAL(triggered()),this,SIGNAL(actionErrors_triggered()));
}

ToolbarWindow::~ToolbarWindow() {
    delete ui;
}

void ToolbarWindow::showEvent(QShowEvent * event) {
    QMainWindow::showEvent(event);
    setMinimumHeight(ui->toolBar->geometry().height());
}

void ToolbarWindow::actionCheckUIUpdate(bool flag) {
    ui->actionCheck_for_updates->setEnabled(flag);
}

void ToolbarWindow::actionUpdateUIUpdate(bool flag) {
    ui->actionUpdate_now->setEnabled(flag);
}

void ToolbarWindow::actionErrorsUIUpdate(bool flag) {
    ui->actionErrors->setEnabled(flag);
}
