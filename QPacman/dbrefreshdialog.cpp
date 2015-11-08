/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "dbrefreshdialog.h"

DBRefreshDialog::DBRefreshDialog(QWidget *parent) : QProgressDialog(parent) {
    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("Please wait..."));
    setLabelText(tr("Synchronizing package databases..."));
    setMinimumDuration(500);
    setRange(0,0);
    connect(this,SIGNAL(canceled()),this,SLOT(onCancel()));

    dbRefresher = new PacmanDBRefresher();
    connect(dbRefresher,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(read_info_finished(PacmanProcessReader *)));
}

void DBRefreshDialog::read_info_finished(PacmanProcessReader * reader) {
    reader->deleteLater();
    cancel();
}

void DBRefreshDialog::onCancel() {
    dbRefresher->terminate();
}
