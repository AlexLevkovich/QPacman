/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "messagedialog.h"
#include "singleapplication.h"
#include "windowcenterer.h"
#include <QEvent>
#include <QTextEdit>
#include <QPushButton>
#include <QMainWindow>

MessageDialog::MessageDialog(const QString& label,const QString & log,const QString & title) : QMessageBox(nullptr) {
    init(label,log,title,false);
}

MessageDialog::MessageDialog(const QString& label,const QString & log,const QString & title,bool is_error) : QMessageBox(nullptr) {
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

    new WindowCenterer(this);
    wnd_flags = windowFlags();
}

MessageDialog * MessageDialog::post(const QString& messagelabel,const QString & log,bool is_error,const QString & title) {
    MessageDialog * dlg = new MessageDialog(messagelabel,log,title,is_error);
    connect(dlg,&MessageDialog::accepted,dlg,&MessageDialog::deleteLater);
    connect(dlg,&MessageDialog::rejected,dlg,&MessageDialog::deleteLater);
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
                connect(buttons[i],&QPushButton::clicked,this,&MessageDialog::setSizeable,Qt::QueuedConnection);
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
    QMainWindow * wnd = SingleApplication::findMainWindow();
    if (wnd != nullptr) {
        setParent(wnd,wnd_flags);
        setModal(true);
        new WindowCenterer(this);
    }
    QMessageBox::open();
}
