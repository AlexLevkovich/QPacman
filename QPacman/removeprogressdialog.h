/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef REMOVEPROGRESSDIALOG_H
#define REMOVEPROGRESSDIALOG_H

#include <QProgressDialog>
#include "pacmanremovepackagesreader.h"

class RemoveProgressDialog : public QProgressDialog {
    Q_OBJECT
public:
    explicit RemoveProgressDialog(const QStringList & packages,QWidget *parent = 0);

protected:
    void showEvent(QShowEvent * event);

protected slots:
    void removing_packages_finished(PacmanProcessReader * reader);
    void start_removing(const QString & package);
    void onCancel();
    void readyToProcess(qreal total_removed);
    void _hide();

signals:
    void post_messages(const QString & package_name,const QStringList & messages);

private:
    PacmanRemovePackagesReader * remover;
    int index;
    bool wasCanceled;
    bool canBeShown;
};

#endif // REMOVEPROGRESSDIALOG_H
