/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "packagechangesdialog.h"
#include "ui_packagechangesdialog.h"
#include "pacmanentry.h"
#include "byteshumanizer.h"
#include <QTimer>

PackageChangesDialog::PackageChangesDialog(const QStringList & install_packages,const QStringList & remove_packages,double total_installed,double total_removed,QWidget *parent) : QDialog(parent), ui(new Ui::PackageChangesDialog) {
    ui->setupUi(this);

#if QT_VERSION >= 0x050000
    ui->packagesList->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->packagesList->header()->setSectionResizeMode(1,QHeaderView::Stretch);    
#else
    ui->packagesList->header()->setResizeMode(0,QHeaderView::ResizeToContents);
    ui->packagesList->header()->setResizeMode(1,QHeaderView::Stretch);
#endif    
    ui->packagesList->setIconSize(QSize(22,22));

    remove_item = NULL;
    install_item = NULL;
    m_install_packages = install_packages;
    m_remove_packages = remove_packages;

    remove_item = fill(m_remove_packages,tr("The following packages will be removed")+" (-"+BytesHumanizer(total_removed).toString()+")",QIcon(":/pics/package-removed.png"));
    install_item = fill(m_install_packages,(tr("The following packages will be installed")+" (%1%2)").arg((total_installed>0)?"+":"").arg(BytesHumanizer(total_installed).toString()),QIcon(":/pics/package-installed.png"));
}

PackageChangesDialog::~PackageChangesDialog() {
    delete ui;
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

    QString name;
    QString version;
    for (int i=0;i<packages.count();i++) {
        PacmanEntry::parseNameVersion(packages[i],name,version);
        QTreeWidgetItem * item = new QTreeWidgetItem(root_item);
        item->setIcon(0,QIcon(":/pics/package.png"));
        item->setText(0,name);
        item->setText(1,version);
        item->setData(0,Qt::UserRole,QVariant::fromValue((double)-1.0));
    }

    root_item->setExpanded(true);

    return root_item;
}

