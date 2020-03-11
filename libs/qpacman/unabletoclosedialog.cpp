/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "unabletoclosedialog.h"
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMainWindow>
#include "static.h"
#include "libalpm.h"

UnableToCloseDialog::UnableToCloseDialog(QWidget *parent) : QDialog(parent) {
    m_close = false;
}

void UnableToCloseDialog::closeEvent(QCloseEvent *e) {
    if (!m_close) {
        e->ignore();
        QMetaObject::invokeMethod(this,"on_reject_requested",Qt::QueuedConnection);
        return;
    }
    UnableToCloseDialog::closeEvent(e);
}

void UnableToCloseDialog::accept() {
    m_close = true;
    QDialog::accept();
}

void UnableToCloseDialog::done(int code) {
    m_close = true;
    QDialog::done(code);
}

int UnableToCloseDialog::exec() {
    m_close = false;
    return QDialog::exec();
}

void UnableToCloseDialog::open() {
    m_close = false;
    QDialog::open();
}

void UnableToCloseDialog::reject() {
    m_close = true;
    QDialog::reject();
}

void UnableToCloseDialog::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Escape && !m_close) {
        e->ignore();
        QMetaObject::invokeMethod(this,"reject_requested",Qt::QueuedConnection);
        return;
    }
    QDialog::keyPressEvent(e);
}

void UnableToCloseDialog::reject_requested() {
    if (QMessageBox::question(Static::pApp()->findMainWindow(),Static::Warning_Str,Static::TransTerminate_Str,QMessageBox::Yes,QMessageBox::No) != QMessageBox::Yes) return;
    Alpm::instance()->setTerminateFlag();
}
