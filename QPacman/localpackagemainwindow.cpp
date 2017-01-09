/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "localpackagemainwindow.h"
#include "ui_localpackagemainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "pacmanfilepackageinforeader.h"
#include "pacmanentry.h"
#include "pacmansimpleitemmodel.h"
#include "packagechangesdialog.h"
#include "installfilesprogressloop.h"
#include "mainwindow.h"
#include "static.h"
#include "errordialog.h"
#include "messagedialog.h"
#include "pacmanlocalpackageslistdelegate.h"

LocalPackageMainWindow::LocalPackageMainWindow(const QStringList & packages,QWidget *parent) : QMainWindow(parent), ui(new Ui::LocalPackageMainWindow) {
    ui->setupUi(this);

    m_packages = packages;
    disableActions();
    ui->waitView->setVisible(true);
    ui->packageView->setVisible(false);

    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->toolBar->insertWidget(ui->actionInstall,empty);

    QMetaObject::invokeMethod(this,"init",Qt::QueuedConnection);
}

LocalPackageMainWindow::~LocalPackageMainWindow() {
    delete ui;
}

void LocalPackageMainWindow::init() {
    ui->waitView->setVisible(true);
    ui->packageView->setVisible(false);
    disableActions();

    model = new PacmanSimpleItemModel(ui->packageView);
    for (int i=0;i<m_packages.count();i++) {
        PacmanFilePackageInfoReader reader(m_packages[i]);
        reader.waitToComplete();
        if (reader.exitCode() != 0) {
            QMetaObject::invokeMethod(qApp,"quit",Qt::QueuedConnection);
            return;
        }
        model->addRow(reader.info());
    }

    PacmanLocalPackagesListDelegate * delegate = new PacmanLocalPackagesListDelegate(ui->packageView);
    delegate->setModel(model);
    for (int i=0;i<model->columnCount();i++) {
        ui->packageView->setItemDelegateForColumn(i,delegate);
    }

    ui->packageView->setModel(model);
    ui->packageView->header()->setStretchLastSection(false);
#if QT_VERSION >= 0x050000
    ui->packageView->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->packageView->header()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->packageView->header()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
    ui->packageView->header()->setSectionsMovable(false);
#else
    ui->packageView->header()->setResizeMode(0,QHeaderView::ResizeToContents);
    ui->packageView->header()->setResizeMode(1,QHeaderView::Stretch);
    ui->packageView->header()->setResizeMode(2,QHeaderView::ResizeToContents);
    ui->packageView->header()->setMovable(false);    
#endif    
    connect(ui->packageView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),this,SLOT(onSelectionChanged(const QItemSelection &,const QItemSelection &)));

    enableActions();
    ui->waitView->setVisible(false);
    ui->packageView->setVisible(true);
}

void LocalPackageMainWindow::onSelectionChanged(const QItemSelection & /*selected*/,const QItemSelection & /*deselected*/) {
    QModelIndexList selected = ui->packageView->selectionModel()->selectedRows();
    if (selected.count() <= 0) return;

    ui->pacInfoView->clearImageCache();
    ui->pacInfoView->setHtml(model->row(selected[0]).toHtml());
}

void LocalPackageMainWindow::stop_wait_indicator() {
    ui->packageView->setVisible(true);
    ui->waitView->setVisible(false);
}

void LocalPackageMainWindow::start_wait_indicator() {
    ui->packageView->setVisible(false);
    ui->waitView->setVisible(true);
}

void LocalPackageMainWindow::on_actionInstall_triggered() {
    disableActions();

    if (!Static::checkRootAccess()) {
        QMessageBox::critical(this,Static::Error_Str,Static::RootRightsNeeded_Str,QMessageBox::Ok);
        qApp->quit();
        return;
    }

    start_wait_indicator();

    if (m_packages.count() > 0) {
        InstallFilesProgressLoop iprogress_dlg(Static::su_password,m_packages,this);
        connect(&iprogress_dlg,SIGNAL(post_messages(const QString &,const QStringList &)),this,SLOT(add_post_messages(const QString &,const QStringList &)));
        connect(&iprogress_dlg,SIGNAL(showingProvidersList()),this,SLOT(stop_wait_indicator()));
        connect(&iprogress_dlg,SIGNAL(hidingProvidersList()),this,SLOT(start_wait_indicator()));
        connect(&iprogress_dlg,SIGNAL(showingPackageListDlg()),this,SLOT(stop_wait_indicator()));
        connect(&iprogress_dlg,SIGNAL(hidingPackageListDlg()),this,SLOT(start_wait_indicator()));
        connect(&iprogress_dlg,SIGNAL(showingFilesDownloadDlg()),this,SLOT(stop_wait_indicator()));
        if (iprogress_dlg.exec() == QDialog::Accepted) {
            if (m_messages.isEmpty()) QMessageBox::information(this,"Information...",Static::InstalledSuccess_Str);
            else MessageDialog(Static::InstalledSuccess_Str,m_messages,this,tr("Post messages...")).exec();
            qApp->quit();
        }
    }
}

void LocalPackageMainWindow::add_post_messages(const QString & package,const QStringList & messages) {
    m_messages += Static::PostMessages_Str.arg(package)+"\n\n";
    m_messages += messages.join("\n");
    m_messages += '\n';
}

void LocalPackageMainWindow::enableActions() {
    ui->actionInstall->setEnabled(true);
}

void LocalPackageMainWindow::disableActions() {
    ui->actionInstall->setEnabled(false);
}
