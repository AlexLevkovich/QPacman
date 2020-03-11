/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "messagedialog.h"
#include "static.h"
#include "singleapplication.h"
#include <QEvent>
#include <QTextEdit>
#include <QPushButton>
#include <QMainWindow>

MessageDialog::MessageDialog(const QString& label,const QString & log,const QString & title) : QMessageBox(NULL) {
    init(label,log,title,false);
}

MessageDialog::MessageDialog(const QString& label,const QString & log,const QString & title,bool is_error) : QMessageBox(NULL) {
    init(label,log,title,is_error);
}

void MessageDialog::init(const QString& label,const QString & log,const QString & title,bool is_error) {
    firstTime = true;

    if (title.isEmpty()) setWindowTitle(is_error?tr("Alpm engine's error"):tr("Alpm engine's messages"));
    else setWindowTitle(title);

    setIcon(is_error?QMessageBox::Critical:QMessageBox::Information);
    setText(label);
    if (!log.isEmpty()) setDetailedText(log);
    setStandardButtons(QMessageBox::Ok);
    setDefaultButton(QMessageBox::Ok);
    setMouseTracking(true);
    setSizeGripEnabled(true);

    Static::makeCentered(this);
    wnd_flags = windowFlags();
}

MessageDialog * MessageDialog::post(const QString& messagelabel,const QString & log,bool is_error,const QString & title) {
    MessageDialog * dlg = new MessageDialog(messagelabel,log,title,is_error);
    connect(dlg,SIGNAL(accepted()),dlg,SLOT(deleteLater()));
    connect(dlg,SIGNAL(rejected()),dlg,SLOT(deleteLater()));
    QMetaObject::invokeMethod(dlg,"open",Qt::QueuedConnection);
    return dlg;
}

void MessageDialog::setSizeable() {
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    QWidget *textEdit = findChild<QTextEdit *>();
    if (textEdit) textEdit->setMaximumHeight(QWIDGETSIZE_MAX);
    updateGeometry();
}

bool MessageDialog::event(QEvent *e) {
    bool res = QMessageBox::event(e);
    switch (e->type()) {
    case QEvent::Show:
    {
        if (firstTime) {
            firstTime = false;
            QList<QPushButton *> buttons = findChildren<QPushButton *>();
            for (int i=0;i<buttons.count();i++) {
                connect(buttons[i],SIGNAL(clicked()),this,SLOT(setSizeable()),Qt::QueuedConnection);
            }
        }
        setSizeable();
        break;
    }
    default:
        break;
    }
    return res;
}

MessageDialog * MessageDialog::post(const QString& messagelabel,const QString & log,const QString & title) {
    return post(messagelabel,log,false,title);
}

ErrorDialog::ErrorDialog(const QString& label,const QString & log,const QString & title) : MessageDialog(label,log,title,true){}

MessageDialog * ErrorDialog::post(const QString& messagelabel,const QString & log,const QString & title) {
    return MessageDialog::post(messagelabel,log,true,title);
}

void MessageDialog::open() {
    QMainWindow * wnd = NULL;
    SingleApplication * pApp = Static::pApp();
    if (pApp != NULL) wnd  = pApp->findMainWindow();
    if (wnd != NULL) {
        setParent(wnd,wnd_flags);
        setModal(true);
        Static::makeCentered(this);
    }
    QMessageBox::open();
}
