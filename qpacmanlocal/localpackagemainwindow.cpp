/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "localpackagemainwindow.h"
#include "ui_localpackagemainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "pacmansimpleitemmodel.h"
#include "packagechangesdialog.h"
#include "static.h"
#include "messagedialog.h"
#include "themeicons.h"
#include "pacmanlocalpackageslistdelegate.h"
#include "packageinstaller.h"
#include "widgetgroup.h"
#include "libalpm.h"
#include <QCheckBox>
#include <QShowEvent>
#include <QCloseEvent>
#include <QDebug>

LocalPackageMainWindow::LocalPackageMainWindow(const QStringList & packages,QWidget *parent) : QMainWindow(parent), ui(new Ui::LocalPackageMainWindow) {
    ui->setupUi(this);

    wasInit = false;
    ui->progressView->setClearOnHide(false);

    view_group = new WidgetGroup(this);
    view_group->add(ui->splitter);
    view_group->add(ui->progressView);
    view_group->add(ui->waitView);
    view_group->add(ui->logWindow);

    ui->actionInstall->setIcon(ThemeIcons::get(ThemeIcons::INSTALL));
    ui->actionCancel->setIcon(ThemeIcons::get(ThemeIcons::CANCEL));

    m_packages = packages;
    ui->actionInstall->setEnabled(false);
    view_group->setCurrent(ui->waitView);

    QWidget* empty = new QWidget(ui->toolBar);
    QVBoxLayout * layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    depsInstall = new QCheckBox(tr("Install all as dependencies"),empty);
    forceInstall = new QCheckBox(tr("Overwrite the files"),empty);
    layout->addWidget(depsInstall);
    layout->addWidget(forceInstall);
    empty->setLayout(layout);
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->toolBar->insertWidget(ui->actionCancel,empty);

    connect(ui->actionInstall,&QAction::triggered,this,&LocalPackageMainWindow::onActionInstall);

    QMetaObject::invokeMethod(this,"init",Qt::QueuedConnection);
}

LocalPackageMainWindow::~LocalPackageMainWindow() {
    delete ui;
}

void LocalPackageMainWindow::init() {
    if (Alpm::instance() == NULL) {
        qApp->quit();
        return;
    }

    model = new PacmanSimpleItemModel(ui->packageView);
    for (int i=0;i<m_packages.count();i++) {
        AlpmPackage pkg(m_packages[i],true);
        if (pkg.isValid()) model->addRow(pkg);
    }
    ui->actionInstall->setEnabled(model->rowCount() > 0);

    PacmanLocalPackagesListDelegate * delegate = new PacmanLocalPackagesListDelegate(ui->packageView);
    for (int i=0;i<model->columnCount();i++) {
        ui->packageView->setItemDelegateForColumn(i,delegate);
    }

    ui->packageView->setModel(model);
    ui->packageView->header()->setStretchLastSection(false);
    ui->packageView->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->packageView->header()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->packageView->header()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
    ui->packageView->header()->setSectionsMovable(false);

    connect(ui->packageView->selectionModel(),&QItemSelectionModel::selectionChanged,this,&LocalPackageMainWindow::onSelectionChanged);

    view_group->setCurrent(ui->splitter);
}

void LocalPackageMainWindow::onSelectionChanged(const QItemSelection & /*selected*/,const QItemSelection & /*deselected*/) {
    QModelIndexList selected = ui->packageView->selectionModel()->selectedRows();
    if (selected.count() <= 0) return;

    ui->pacInfoView->fillByInfo(model->row(selected[0]),true);
}

void LocalPackageMainWindow::onActionInstall() {
    ui->actionInstall->setEnabled(false);
    view_group->setCurrent(ui->progressView);

    if (model->rowCount() > 0) {
        ui->actionCancel->setVisible(true);
        ui->actionInstall->setVisible(false);
        depsInstall->setVisible(false);
        forceInstall->setVisible(false);
        PackageInstaller * installer = new PackageInstaller(model->allrows(),forceInstall->isChecked()?model->allrows():QList<AlpmPackage>(),depsInstall->isChecked(),ui->progressView,ui->actionCancel,NULL,NULL);
        connect(installer,&PackageInstaller::completed,this,&LocalPackageMainWindow::operation_completed);
        connect(installer,&PackageInstaller::logString,this,&LocalPackageMainWindow::logString);
    }
}

void LocalPackageMainWindow::showEvent(QShowEvent * event) {
    QMainWindow::showEvent(event);
    if (!wasInit) {
        wasInit = true;
        QSettings settings;
        restoreGeometry(settings.value("geometry/mainwindow").toByteArray());
        restoreState(settings.value("state/mainwindow").toByteArray());
        ui->splitter->restoreState(settings.value("state/splitter").toByteArray());
    }
}

void LocalPackageMainWindow::closeEvent(QCloseEvent * event) {
    QSettings settings;
    settings.setValue("geometry/mainwindow",saveGeometry());
    settings.setValue("state/mainwindow",saveState());
    settings.setValue("state/splitter",ui->splitter->saveState());

    QMainWindow::closeEvent(event);
}

void LocalPackageMainWindow::operation_completed(ThreadRun::RC rc,const QString &) {
    if (rc == ThreadRun::OK) model->forbidRowDeletions();
    ui->actionCancel->setVisible(true);
    ui->actionCancel->setEnabled(true);
    ui->actionLog->setVisible(true);
    connect(ui->actionCancel,&QAction::triggered,this,&LocalPackageMainWindow::close);
    ui->actionCancel->setText(tr("Quit"));
}

void LocalPackageMainWindow::logString(const QString & str) {
    ui->logWindow->appendPlainText(str.endsWith("\n")?str.left(str.length()-1):str);
    ui->logWindow->ensureCursorVisible();
}

void LocalPackageMainWindow::on_actionLog_triggered(bool checked) {
    view_group->setCurrent(checked?(QWidget *)ui->logWindow:(QWidget *)ui->progressView);
}
