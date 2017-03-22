/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "messagedialog.h"
#include "static.h"
#include <QApplication>

MessageDialog::MessageDialog(const QString& label,const QString & log,QWidget *parent,const QString & title) : QMessageBox(parent) {
    if (title.isEmpty()) setWindowTitle(tr("Pacman's message"));
    else setWindowTitle(title);

    setIcon(QMessageBox::Information);
    setText(label);
    if (!log.isEmpty()) setDetailedText(log);
    setStandardButtons(QMessageBox::Ok);
    setDefaultButton(QMessageBox::Ok);

    Static::makeCentered(this);
}

MessageDialog::MessageDialog(const QString& label,const QString & log,const QString & title,bool is_error,QWidget *parent) : QMessageBox(parent) {
    if (title.isEmpty()) setWindowTitle(is_error?tr("Pacman's error"):tr("Pacman's message"));
    else setWindowTitle(title);

    setIcon(is_error?QMessageBox::Critical:QMessageBox::Information);
    setText(label);
    if (!log.isEmpty()) setDetailedText(log);
    setStandardButtons(QMessageBox::Ok);
    setDefaultButton(QMessageBox::Ok);

    Static::makeCentered(this);
}

void MessageDialog::post(const QString& messagelabel,const QString & log,bool is_error,const QString & title) {
    MessageDialog * dlg = new MessageDialog(messagelabel,log,title,is_error,Static::findMainWindow());
    connect(dlg,SIGNAL(accepted()),dlg,SLOT(deleteLater()));
    connect(dlg,SIGNAL(rejected()),dlg,SLOT(deleteLater()));
    QMetaObject::invokeMethod(dlg,"exec",Qt::QueuedConnection);
}

void MessageDialog::post(const QString& messagelabel,const QString & log,const QString & title) {
    post(messagelabel,log,false,title);
}

ErrorDialog::ErrorDialog(const QString& label,const QString & log,QWidget *parent,const QString & title) : MessageDialog(label,log,title,true,parent){}

void ErrorDialog::post(const QString& messagelabel,const QString & log,const QString & title) {
    MessageDialog::post(messagelabel,log,true,title);
}
