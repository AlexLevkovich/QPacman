/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACKAGEPROVIDERSDIALOG_H
#define PACKAGEPROVIDERSDIALOG_H

#include "unabletoclosedialog.h"
#include <QModelIndex>

namespace Ui {
class PackageProvidersDialog;
}

class PackageProvidersDialog : public UnableToCloseDialog {
    Q_OBJECT

public:
    explicit PackageProvidersDialog();
    ~PackageProvidersDialog();

protected slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_packagesList_doubleClicked(const QModelIndex &index);
    void select_provider(const QString & pkgname,const QStringList & providers);
    void reject_requested();

private:
    void fill(const QStringList & m_packages);
    uint provider() const;

    Ui::PackageProvidersDialog *ui;
};

#endif // PackageProvidersDialog_H
