/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef INSTALLPROGRESSDIALOG_H
#define INSTALLPROGRESSDIALOG_H

#include <QProgressDialog>
#include "libqpacman_global.h"

class QKeyEvent;
class PacmanUpdatePackagesReader;

class LIBQPACMANSHARED_EXPORT InstallProgressDialog : public QProgressDialog {
    Q_OBJECT
public:
    explicit InstallProgressDialog(PacmanUpdatePackagesReader * installer,uint pkgs_count,QWidget *parent = 0);

protected slots:
    void start_installing(const QString & package);
    void start_removing(const QString & package);

protected:
    void keyPressEvent(QKeyEvent *event);

    PacmanUpdatePackagesReader * installer;

private:
    int index;
    bool first_pkg;
    uint pkgs_count;
};

#endif // INSTALLPROGRESSDIALOG_H
