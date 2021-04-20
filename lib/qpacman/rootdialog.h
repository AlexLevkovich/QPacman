/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef ROOTDIALOG_H
#define ROOTDIALOG_H

#include "unabletoclosedialog.h"

namespace Ui {
class RootDialog;
}

class PacmanProcessReader;

class RootDialog : public UnableToCloseDialog {
    Q_OBJECT

public:
    explicit RootDialog();
    ~RootDialog();
    QString password() const;

protected slots:
    void reject_requested();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_passEdit_textChanged(const QString &arg1);

private:
    Ui::RootDialog *ui;
};

#endif // ROOTDIALOG_H
