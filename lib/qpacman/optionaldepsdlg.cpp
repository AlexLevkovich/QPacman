/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "optionaldepsdlg.h"
#include "ui_optionaldepsdlg.h"
#include "alpmpackage.h"
#include "themeicons.h"
#include "singleapplication.h"
#include <QPushButton>
#include <QMetaMethod>
#include <QMainWindow>
#include <QDebug>

OptionalDepsDlg::OptionalDepsDlg() : QDialog(SingleApplication::findMainWindow()), ui(new Ui::OptionalDepsDlg) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose,true);
    ui->depsWidget->setColumnHidden(2,true);
    m_opt_deps_enabled = false;
    m_processing_completed = false;

    connect(ui->depsWidget->model(),&QAbstractItemModel::rowsInserted,this,&OptionalDepsDlg::rowsChanged);
    connect(ui->depsWidget->model(),&QAbstractItemModel::rowsRemoved,this,&OptionalDepsDlg::rowsChanged);
}

OptionalDepsDlg::~OptionalDepsDlg() {
    delete ui;
}

void OptionalDepsDlg::setProcessingEnabled() {
    m_processing_completed = true;
    on_depsWidget_itemChanged(nullptr,0);
}

void OptionalDepsDlg::rowsChanged() {
    for(int i = 0; i < 4; i++) ui->depsWidget->resizeColumnToContents(i);
}

QStringList OptionalDepsDlg::listedPackages() const {
    QStringList ret;
    QTreeWidgetItem * item;
    for (int i=0;i<ui->depsWidget->model()->rowCount();i++) {
        item = ui->depsWidget->topLevelItem(i);
        if (!item->text(1).isEmpty()) ret.append(item->text(1));
    }
    return ret;
}

void OptionalDepsDlg::fill(const QString & pkgname,const StringStringMap & installed_deps,const StringStringMap & pending_deps) {
    struct DescState {
        bool installed;
        QString desc;
        QString version;
    };

    QString name;
    QString version;
    bool all_versions_empty = true;
    DescState desc_state;
    QMap<QString,DescState> deps;
    QStringList listed_pkgs = listedPackages();

    desc_state.installed = true;
    for(QString & key: installed_deps.keys()) {
        AlpmPackage::parseNameVersion(key,name,version);
        if (listed_pkgs.contains(name)) continue;
        if (!version.isEmpty()) all_versions_empty = false;
        desc_state.desc = installed_deps[key];
        desc_state.version = version;
        deps.insert(name,desc_state);
    }

    desc_state.installed = false;
    for(QString & key: pending_deps.keys()) {
        AlpmPackage::parseNameVersion(key,name,version);
        if (listed_pkgs.contains(name)) continue;
        if (!version.isEmpty()) all_versions_empty = false;
        desc_state.desc = pending_deps[key];
        desc_state.version = version;
        deps.insert(name,desc_state);
    }

    if (deps.isEmpty()) return;

    QTreeWidgetItem * item = new QTreeWidgetItem(ui->depsWidget);
    item->setFlags(Qt::ItemIsEnabled);
    item->setFirstColumnSpanned(true);
    item->setText(0,pkgname);
    item->setIcon(0,ThemeIcons::get(ThemeIcons::PKG));
    item->setBackground(0,palette().brush(QPalette::Button));
    for(QString & key: deps.keys()) {
        item = new QTreeWidgetItem(ui->depsWidget,QStringList() << QString() << key << deps[key].version << deps[key].desc);
        if (!deps[key].installed) item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        item->setCheckState(0,deps[key].installed?Qt::Checked:Qt::Unchecked);
        item->setDisabled(deps[key].installed);
        item->setIcon(1,deps[key].installed?ThemeIcons::get(ThemeIcons::PKG_INSTALLED_MARK):ThemeIcons::get(ThemeIcons::PKG_NONINSTALLED_MARK));
    }

    ui->depsWidget->setColumnHidden(2,all_versions_empty && ui->depsWidget->isColumnHidden(2));
    ui->depsWidget->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    if (!listedPackages().isEmpty() && !isVisible()) open();
}

void OptionalDepsDlg::on_buttonBox_clicked(QAbstractButton *button) {
    switch (ui->buttonBox->buttonRole(button)) {
    case QDialogButtonBox::RejectRole:
        reject();
        break;
    case QDialogButtonBox::ApplyRole:
        emit_selected();
        accept();
        break;
    default:
        break;
    }
}

void OptionalDepsDlg::on_depsWidget_itemChanged(QTreeWidgetItem *, int column) {
    if (column != 0) return;

    bool checked = false;
    QTreeWidgetItem * item;
    for (int i=0;i<ui->depsWidget->model()->rowCount();i++) {
        item = ui->depsWidget->topLevelItem(i);
        if ((item->checkState(0) == Qt::Checked) && !item->isDisabled()) {
            checked = true;
            break;
        }
    }
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(checked && m_processing_completed);
}

void OptionalDepsDlg::emit_selected() {
    QStringList ret;
    QTreeWidgetItem * item;
    for (int i=0;i<ui->depsWidget->model()->rowCount();i++) {
        item = ui->depsWidget->topLevelItem(i);
        if ((item->checkState(0) == Qt::Checked) && !item->isDisabled()) ret.append(item->text(1));
    }
    if(ret.count() > 0) emit selected(ret);
}
