/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QMessageBox>

class MessageDialog : public QMessageBox {
    Q_OBJECT

public:
    explicit MessageDialog(const QString& label,const QString & log,QWidget *parent = 0,const QString & title = tr("Pacman's message"));
    ~MessageDialog();
};

#endif // MESSAGEDIALOG_H
