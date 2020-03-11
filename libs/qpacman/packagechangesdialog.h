/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACKAGECHANGESDIALOG_H
#define PACKAGECHANGESDIALOG_H

#include "unabletoclosedialog.h"

namespace Ui {
class PackageChangesDialog;
}

class QTreeWidgetItem;

class PackageChangesDialog : public UnableToCloseDialog {
    Q_OBJECT

public:
    PackageChangesDialog();
    ~PackageChangesDialog();

protected slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void reject_requested();
    void install_packages_confirmation(const QStringList & install,const QStringList & remove,qint64 dl_size,qint64 install_size,qint64 remove_size,bool * answer);
    void remove_packages_confirmation(const QStringList & remove,qint64 remove_size,bool * answer);

private:
    Ui::PackageChangesDialog *ui;

    QTreeWidgetItem * fill(const QStringList & packages,const QString & labelString,const QIcon & icon);
};

#endif // PACKAGECHANGESDIALOG_H
