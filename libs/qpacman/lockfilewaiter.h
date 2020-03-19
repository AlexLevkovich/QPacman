/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef LOCKFILEWAITER_H
#define LOCKFILEWAITER_H

#include <QDialog>
#include "inotifier.h"

namespace Ui {
class LockFileWaiter;
}

class LockFileWaiter : public QDialog {
    Q_OBJECT

public:
    explicit LockFileWaiter(const QStringList & files,const QString & infoText = QString(),bool show_cancel_noquit = false,QWidget *parent = nullptr);
    ~LockFileWaiter();

protected:
    void showEvent(QShowEvent * event);

private slots:
    void pathRejected(const QString & path,Inotifier::Error error);
    void on_deleteButton_clicked();
    void on_cancelButton_clicked();
    void on_quitButton_clicked();
    void on_filesList_itemSelectionChanged();

private:
    Ui::LockFileWaiter *ui;
    Inotifier fw;
};

#endif // LOCKFILEWAITER_H
