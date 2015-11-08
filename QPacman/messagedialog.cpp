/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "messagedialog.h"

MessageDialog::MessageDialog(const QString& label,const QString & log,QWidget *parent,const QString & title) : QMessageBox(parent) {
    setWindowTitle(title);

    setIcon(QMessageBox::Information);
    setText(label);
    setDetailedText(log);
    setStandardButtons(QMessageBox::Ok);
    setDefaultButton(QMessageBox::Ok);
}

MessageDialog::~MessageDialog() {
}

