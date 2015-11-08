/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanhelpdialog.h"
#include "ui_pacmanhelpdialog.h"

PacmanHelpDialog::PacmanHelpDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PacmanHelpDialog) {
    ui->setupUi(this);
}

PacmanHelpDialog::~PacmanHelpDialog() {
    delete ui;
}
