/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACKAGECHANGESDIALOG_H
#define PACKAGECHANGESDIALOG_H

#include <QDialog>
#include "libqpacman_global.h"

namespace Ui {
class PackageChangesDialog;
}

class QTreeWidgetItem;

class LIBQPACMANSHARED_EXPORT PackageChangesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PackageChangesDialog(const QStringList & install_packages,const QStringList & remove_packages,double total_installed,double total_removed,QWidget *parent = 0);
    ~PackageChangesDialog();

protected slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::PackageChangesDialog *ui;
    QTreeWidgetItem * remove_item;
    QTreeWidgetItem * install_item;
    QStringList m_install_packages;
    QStringList m_remove_packages;

    QTreeWidgetItem * fill(const QStringList & packages,const QString & labelString,const QIcon & icon);
};

#endif // PACKAGECHANGESDIALOG_H