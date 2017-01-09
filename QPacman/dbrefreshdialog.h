/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef DBREFRESHDIALOG_H
#define DBREFRESHDIALOG_H

#include <QProgressDialog>
#include "pacmandbrefresher.h"

class QEvent;

class DBRefreshDialog : public QProgressDialog {
    Q_OBJECT
public:
    DBRefreshDialog(QWidget *parent = 0);
    int exec();

protected slots:
    void read_info_finished(PacmanProcessReader * reader);
    void onCancel();

private:
    PacmanDBRefresher * dbRefresher;
    bool m_ok;
};

#endif // DBREFRESHDIALOG_H
