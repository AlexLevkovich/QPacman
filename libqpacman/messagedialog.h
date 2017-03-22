/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QMessageBox>
#include "libqpacman_global.h"

class LIBQPACMANSHARED_EXPORT MessageDialog : public QMessageBox {
    Q_OBJECT

public:
    MessageDialog(const QString& label,const QString & log,QWidget *parent = 0,const QString & title = QString());
    static void post(const QString& messagelabel,const QString & log,const QString & title = QString());

protected:
    MessageDialog(const QString& label,const QString & log,const QString & title,bool is_error = false,QWidget *parent = 0);
    static void post(const QString& messagelabel,const QString & log,bool is_error = false,const QString & title = QString());
};

class LIBQPACMANSHARED_EXPORT ErrorDialog : public MessageDialog {
public:
    ErrorDialog(const QString& label,const QString & log,QWidget *parent = 0,const QString & title = QString());
    static void post(const QString& messagelabel,const QString & log,const QString & title = QString());
};

#endif // MESSAGEDIALOG_H
