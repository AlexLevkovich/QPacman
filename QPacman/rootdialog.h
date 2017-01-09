/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef ROOTDIALOG_H
#define ROOTDIALOG_H

#include <QDialog>

namespace Ui {
class RootDialog;
}

class PacmanProcessReader;

class RootDialog : public QDialog {
    Q_OBJECT

public:
    explicit RootDialog(QWidget *parent = 0);
    ~RootDialog();
    QString password() const;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::RootDialog *ui;
};

#endif // ROOTDIALOG_H
