/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef REMOVEPROGRESSDIALOG_H
#define REMOVEPROGRESSDIALOG_H

#include <QProgressDialog>
#include "pacmanremovepackagesreader.h"

class LIBQPACMANSHARED_EXPORT RemoveProgressDialog : public QProgressDialog {
    Q_OBJECT
public:
    explicit RemoveProgressDialog(PacmanRemovePackagesReader * remover,QWidget *parent = 0);
    QStringList removedPackages() const;

protected:
    void keyPressEvent(QKeyEvent *event);

protected slots:
    void start_removing(const QString & package);

private:
    PacmanRemovePackagesReader * remover;
    int index;
    QStringList removed_packages;
};

#endif // REMOVEPROGRESSDIALOG_H
