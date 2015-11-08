/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QMessageBox>

class ErrorDialog : public QMessageBox {
    Q_OBJECT

public:
    explicit ErrorDialog(const QString& errorlabel,const QString & log,QWidget *parent = 0,const QString & title = tr("Pacman's error"));
    ~ErrorDialog();
};

#endif // ERRORDIALOG_H
