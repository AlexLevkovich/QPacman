/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "dbrefreshdialog.h"
#include "static.h"
#include <QMessageBox>
#include <QShowEvent>

DBRefreshDialog::DBRefreshDialog(QWidget *parent) : QProgressDialog(parent) {
    setWindowTitle(tr("Please wait..."));
    setLabelText(tr("Synchronizing package databases..."));
    setMinimumDuration(500);
    setRange(0,0);
    connect(this,SIGNAL(canceled()),this,SLOT(onCancel()));

    dbRefresher = NULL;
    m_ok = false;

    if (!Static::checkRootAccess()) {
        QMessageBox::critical(this,Static::Error_Str,Static::RootRightsNeeded_Str,QMessageBox::Ok);
        return;
    }

    m_ok = true;

    dbRefresher = new PacmanDBRefresher(Static::su_password,this);
    connect(dbRefresher,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(read_info_finished(PacmanProcessReader *)));
}

int DBRefreshDialog::exec() {
    if (!m_ok) return QDialog::Rejected;
    return QDialog::exec();
}

void DBRefreshDialog::read_info_finished(PacmanProcessReader * reader) {
    reader->deleteLater();
    dbRefresher = NULL;
    cancel();
}

void DBRefreshDialog::onCancel() {
    if (dbRefresher != NULL) dbRefresher->terminate();
}
