/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef ROOTDIALOG_H
#define ROOTDIALOG_H

class QEvent;

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

protected:
    bool event(QEvent *e);

protected slots:
    void reject_requested();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::RootDialog *ui;
    bool firstTime;
};

#endif // ROOTDIALOG_H
