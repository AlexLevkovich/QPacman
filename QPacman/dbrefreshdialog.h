/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef DBREFRESHDIALOG_H
#define DBREFRESHDIALOG_H

#include <QProgressDialog>
#include "pacmandbrefresher.h"

class DBRefreshDialog : public QProgressDialog {
    Q_OBJECT
public:
    explicit DBRefreshDialog(QWidget *parent = 0);

protected slots:
    void read_info_finished(PacmanProcessReader * reader);
    void onCancel();

private:
    PacmanDBRefresher * dbRefresher;
};

#endif // DBREFRESHDIALOG_H
