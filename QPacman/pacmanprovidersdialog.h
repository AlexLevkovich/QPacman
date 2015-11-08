/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANPROVIDERSDIALOG_H
#define PACMANPROVIDERSDIALOG_H

#include <QDialog>
#include <QModelIndex>

namespace Ui {
class PacmanProvidersDialog;
}

class PacmanProvidersDialog : public QDialog {
    Q_OBJECT

public:
    explicit PacmanProvidersDialog(const QStringList & packages,QWidget *parent = 0);
    ~PacmanProvidersDialog();
    QString provider() const;

protected slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_packagesList_doubleClicked(const QModelIndex &index);

private:
    void fill();

    Ui::PacmanProvidersDialog *ui;
    QStringList m_packages;
};

#endif // PACMANPROVIDERSDIALOG_H
