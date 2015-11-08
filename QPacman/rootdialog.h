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
    explicit RootDialog(const QByteArray & salt,QWidget *parent = 0);
    ~RootDialog();
    bool passwordIsOK() { return m_passwordIsOK; }

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::RootDialog *ui;

    QByteArray m_salt;
    bool m_passwordIsOK;
};

#endif // ROOTDIALOG_H
