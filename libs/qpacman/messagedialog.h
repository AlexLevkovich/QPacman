/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QMessageBox>

class QEvent;

class MessageDialog : public QMessageBox {
    Q_OBJECT

public:
    MessageDialog(const QString& label,const QString & log,const QString & title = QString());
    static MessageDialog * post(const QString& messagelabel,const QString & log,const QString & title = QString());
    static MessageDialog * post(const QString& messagelabel,const QString & log,bool is_error = false,const QString & title = QString());

protected:
    MessageDialog(const QString& label,const QString & log,const QString & title,bool is_error = false);
    bool event(QEvent *e);

public slots:
    virtual void open();

private slots:
    void setSizeable();

private:
    void init(const QString& label,const QString & log,const QString & title,bool is_error);

    Qt::WindowFlags wnd_flags;
    bool firstTime;
};

class ErrorDialog : public MessageDialog {
public:
    ErrorDialog(const QString& label,const QString & log,const QString & title = QString());
    static MessageDialog * post(const QString& messagelabel,const QString & log,const QString & title = QString());
};

#endif // MESSAGEDIALOG_H
