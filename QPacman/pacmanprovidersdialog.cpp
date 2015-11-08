/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanprovidersdialog.h"
#include "ui_pacmanprovidersdialog.h"

PacmanProvidersDialog::PacmanProvidersDialog(const QStringList & packages,QWidget *parent) : QDialog(parent), ui(new Ui::PacmanProvidersDialog) {
    ui->setupUi(this);
    m_packages = packages;

#if QT_VERSION >= 0x050000
    ui->packagesList->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
#else
    ui->packagesList->header()->setResizeMode(0,QHeaderView::ResizeToContents);
#endif 
    ui->packagesList->setIconSize(QSize(32,32));

    fill();
}

PacmanProvidersDialog::~PacmanProvidersDialog() {
    delete ui;
}

void PacmanProvidersDialog::fill() {
    QTreeWidgetItem * firstitem = NULL;
    for (int i=0;i<m_packages.count();i++) {
        QTreeWidgetItem * item = new QTreeWidgetItem(ui->packagesList);
        item->setIcon(0,QIcon(":/pics/package.png"));
        item->setText(0,m_packages[i]);
        if (i == 0) firstitem = item;
    }

    if (firstitem != NULL) {
        ui->packagesList->setCurrentItem(firstitem);
    }
}

void PacmanProvidersDialog::on_buttonBox_accepted() {
    accept();
}

void PacmanProvidersDialog::on_buttonBox_rejected() {
    reject();
}

QString PacmanProvidersDialog::provider() const {
    if (ui->packagesList->currentItem() != NULL) {
        return ui->packagesList->currentItem()->text(0);
    }
    return QString();
}

void PacmanProvidersDialog::on_packagesList_doubleClicked(const QModelIndex &/*index*/) {
    accept();
}
