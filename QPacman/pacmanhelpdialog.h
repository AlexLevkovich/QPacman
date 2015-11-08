/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANHELPDIALOG_H
#define PACMANHELPDIALOG_H

#include <QDialog>

namespace Ui {
class PacmanHelpDialog;
}

class PacmanHelpDialog : public QDialog {
    Q_OBJECT
public:
    explicit PacmanHelpDialog(QWidget *parent = 0);
    ~PacmanHelpDialog();

private:
    Ui::PacmanHelpDialog *ui;
};

#endif // PACMANHELPDIALOG_H
