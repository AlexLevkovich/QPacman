/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "errordialog.h"

ErrorDialog::ErrorDialog(const QString& errorlabel,const QString & log,QWidget *parent,const QString & title) : QMessageBox(parent) {
    setWindowTitle(title);

    setIcon(QMessageBox::Critical);
    setText(errorlabel);
    setDetailedText(log);
    setStandardButtons(QMessageBox::Ok);
    setDefaultButton(QMessageBox::Ok);
}

ErrorDialog::~ErrorDialog() {
}

