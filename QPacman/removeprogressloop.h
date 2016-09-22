/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef REMOVEPROGRESSLOOP_H
#define REMOVEPROGRESSLOOP_H

#include <QEventLoop>
#include "pacmanremovepackagesreader.h"

class RemoveProgressLoop : public QEventLoop {
    Q_OBJECT
public:
    explicit RemoveProgressLoop(const QStringList & packages,bool withDeps = true,QWidget *parent = 0);
    QStringList removedPackages() const;

protected slots:
    void removing_packages_finished(PacmanProcessReader * reader);
    void onCancel();
    void readyToProcess(double total_removed);

signals:
    void post_messages(const QString & package_name,const QStringList & messages);

private:
    PacmanRemovePackagesReader * remover;
    bool wasCanceled;
    QStringList removed_packages;
};

#endif // REMOVEPROGRESSDIALOG_H
