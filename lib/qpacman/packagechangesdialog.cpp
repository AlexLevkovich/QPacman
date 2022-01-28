/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "packagechangesdialog.h"
#include "ui_packagechangesdialog.h"
#include "alpmpackage.h"
#include "byteshumanizer.h"
#include "windowcenterer.h"
#include "singleapplication.h"
#include "themeicons.h"
#include "static.h"
#include "libalpm.h"
#include "themeicons.h"
#include <QTimer>
#include <QMainWindow>

PackageChangesDialog::PackageChangesDialog() : UnableToCloseDialog(SingleApplication::findMainWindow()), ui(new Ui::PackageChangesDialog) {
    ui->setupUi(this);

    setWindowIcon(ThemeIcons::get(ThemeIcons::CHANGES_DLG));

    new WindowCenterer(this);

    connect(Alpm::instance(),&Alpm::install_packages_confirmation,this,&PackageChangesDialog::install_packages_confirmation);
    connect(Alpm::instance(),&Alpm::remove_packages_confirmation,this,&PackageChangesDialog::remove_packages_confirmation);
}

PackageChangesDialog::~PackageChangesDialog() {
    delete ui;
}

void PackageChangesDialog::install_packages_confirmation(const QStringList & install,const QStringList & remove,qint64,qint64 install_size,qint64 remove_size) {
    ui->packagesList->clear();
    fill(remove,tr("The following packages will be removed")+" (-"+BytesHumanizer(remove_size).toString()+")",ThemeIcons::get(ThemeIcons::PKG_REMOVED));
    fill(install,(tr("The following packages will be installed")+" (%1%2)").arg((install_size>0)?"+":"").arg(BytesHumanizer(install_size).toString()),ThemeIcons::get(ThemeIcons::PKG_INSTALLED));
    Alpm::instance()->answer(exec() == QDialog::Accepted);
}

void PackageChangesDialog::remove_packages_confirmation(const QStringList & remove,qint64 remove_size) {
    ui->packagesList->clear();
    fill(remove,tr("The following packages will be removed")+" (-"+BytesHumanizer(remove_size).toString()+")",ThemeIcons::get(ThemeIcons::PKG_REMOVED));
    Alpm::instance()->answer(exec() == QDialog::Accepted);
}

void PackageChangesDialog::on_buttonBox_accepted() {
    accept();
}

void PackageChangesDialog::on_buttonBox_rejected() {
    reject();
}

QTreeWidgetItem * PackageChangesDialog::fill(const QStringList & packages,const QString & labelString,const QIcon & icon) {
    if (packages.count() <= 0) return NULL;

    QTreeWidgetItem * root_item = new QTreeWidgetItem(ui->packagesList);
    root_item->setIcon(0,icon);
    root_item->setText(0,labelString);
    QFont font(root_item->font(0));
    font.setBold(true);
    root_item->setFont(0,font);

    QString repo;
    QString name;
    QString version;
    for (int i=0;i<packages.count();i++) {
        AlpmPackage::parseNameVersion(packages[i],repo,name,version);
        QTreeWidgetItem * item = new QTreeWidgetItem(root_item);
        item->setIcon(0,ThemeIcons::get(ThemeIcons::PKG));
        item->setText(0,name);
        item->setText(1,version);
        item->setText(2,repo);
        item->setData(0,Qt::UserRole,QVariant::fromValue((double)-1.0));
    }

    root_item->setExpanded(true);

    return root_item;
}
