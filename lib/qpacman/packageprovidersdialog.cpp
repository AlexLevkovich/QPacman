/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "packageprovidersdialog.h"
#include "libalpm.h"
#include "themeicons.h"
#include "ui_packageprovidersdialog.h"
#include <QMainWindow>
#include "singleapplication.h"
#include "windowcenterer.h"

PackageProvidersDialog::PackageProvidersDialog() : UnableToCloseDialog(SingleApplication::findMainWindow()), ui(new Ui::PackageProvidersDialog) {
    ui->setupUi(this);

    setWindowIcon(ThemeIcons::get(ThemeIcons::PROVIDER_DLG));

    ui->packagesList->setIconSize(QSize(32,32));

    new WindowCenterer(this);

    connect(Alpm::instance(),&Alpm::select_provider,this,&PackageProvidersDialog::select_provider);
}

PackageProvidersDialog::~PackageProvidersDialog() {
    delete ui;
}

void PackageProvidersDialog::select_provider(const QString & pkgname,const QStringList & providers) {
    ui->label->setText("<html><head/><body><p><span style=\" font-weight:600;\">"+tr("Some providers for %1 are available").arg(pkgname)+"</span></p></body></html>");
    fill(providers);
    Alpm::instance()->answer((exec() == QDialog::Accepted)?provider():(uint)0);
}

void PackageProvidersDialog::fill(const QStringList & m_packages) {
    QString repo;
    QString name;
    QString version;
    QTreeWidgetItem * firstitem = nullptr;
    for (int i=0;i<m_packages.count();i++) {
        AlpmPackage::parseNameVersion(m_packages[i],repo,name,version);
        QTreeWidgetItem * item = new QTreeWidgetItem(ui->packagesList);
        item->setIcon(0,ThemeIcons::get(ThemeIcons::PKG));
        item->setText(0,name);
        item->setText(1,version);
        item->setText(2,repo);
        if (i == 0) firstitem = item;
    }

    if (firstitem != nullptr) {
        ui->packagesList->setCurrentItem(firstitem);
    }
    new WindowCenterer(this);
}

void PackageProvidersDialog::on_buttonBox_accepted() {
    accept();
}

void PackageProvidersDialog::on_buttonBox_rejected() {
    reject_requested();
}

uint PackageProvidersDialog::provider() const {
    if (ui->packagesList->currentItem() != nullptr) {
        return ui->packagesList->currentIndex().row();
    }
    return 0;
}

void PackageProvidersDialog::on_packagesList_doubleClicked(const QModelIndex &/*index*/) {
    on_buttonBox_accepted();
}

void PackageProvidersDialog::reject_requested() {
    reject();
}
