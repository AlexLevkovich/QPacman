/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "optionswidget.h"
#include "themeicons.h"
#include <QEvent>
#include <QFontMetrics>
#include <QApplication>
#include <QVariant>
#include <QCheckBox>
#include <QVBoxLayout>
#include "packageview.h"

OptionsWidget::OptionsWidget(QWidget *parent) : QWidget(parent) {
    qApp->installEventFilter(this);
}

uint OptionsWidget::maxTextWidth(const QFont & font) {
    QFontMetrics fm(font);
    return (uint)qMax(fm.horizontalAdvance(PackageItemModel::DO_INSTALL_STR),qMax(fm.horizontalAdvance(PackageItemModel::DO_REINSTALL_STR),fm.horizontalAdvance(PackageItemModel::DO_UNINSTALL_STR)));
}

QString OptionsWidget::currentStatusText() const {
    return PackageItemModel::status_text(currentStatus());
}

QString OptionsWidget::currentStatusTextHint() const {
    return PackageItemModel::statusTextHint(currentStatus());
}

QIcon OptionsWidget::currentStatusIcon() const {
    return PackageItemModel::status_icon(currentStatus());
}

bool OptionsWidget::eventFilter(QObject *obj, QEvent *e) {
    if (obj == this && (e->type() == QEvent::MouseButtonRelease || e->type() == QEvent::MouseButtonPress)) return true;
    return QWidget::eventFilter(obj,e);
}

UninstallOptionsWidget::UninstallOptionsWidget(QWidget *parent) : OptionsWidget(parent) {
    setObjectName(QString::fromUtf8("UninstallOptionsWidget"));
    verticalLayout = new QVBoxLayout(this);
    depsCheck = new QCheckBox(this);
    verticalLayout->addWidget(depsCheck);
    depsCheck->setText(tr("With dependencies"));
    depsCheck->setChecked(true);
}

AlpmPackage::UserChangeStatus UninstallOptionsWidget::currentStatus() const {
    return depsCheck->isChecked()?AlpmPackage::DO_UNINSTALL_ALL:AlpmPackage::DO_UNINSTALL;
}

bool UninstallOptionsWidget::setStatus(AlpmPackage::UserChangeStatus status) {
    if (status != AlpmPackage::DO_UNINSTALL &&
        status != AlpmPackage::DO_UNINSTALL_ALL) return false;
    depsCheck->setChecked(status & AlpmPackage::DO_UNINSTALL_ALL);
    return true;
}

ReinstallOptionsWidget::ReinstallOptionsWidget(QWidget *parent) : OptionsWidget(parent) {
    verticalLayout = new QVBoxLayout(this);
    asDepsCheck = new QCheckBox(this);
    verticalLayout->addWidget(asDepsCheck);
    asDepsCheck->setText(tr("As a dependency"));
    asDepsCheck->setChecked(false);
}

AlpmPackage::UserChangeStatus ReinstallOptionsWidget::currentStatus() const {
    return asDepsCheck->isChecked()?AlpmPackage::DO_REINSTALL_ASDEPS:AlpmPackage::DO_REINSTALL;
}

bool ReinstallOptionsWidget::setStatus(AlpmPackage::UserChangeStatus status) {
    if (status != AlpmPackage::DO_REINSTALL &&
        status != AlpmPackage::DO_REINSTALL_ASDEPS) return false;
    asDepsCheck->setChecked(status & AlpmPackage::DO_REINSTALL_ASDEPS);
    return true;
}

InstallOptionsWidget::InstallOptionsWidget(QWidget *parent) : OptionsWidget(parent) {
    verticalLayout = new QVBoxLayout(this);
    asDepsCheck = new QCheckBox(this);
    verticalLayout->addWidget(asDepsCheck);
    forceCheck = new QCheckBox(this);
    verticalLayout->addWidget(forceCheck);
    asDepsCheck->setText(tr("As a dependency"));
    forceCheck->setText(tr("Overwrite the files"));

    forceCheck->setChecked(false);
    asDepsCheck->setChecked(false);
}

AlpmPackage::UserChangeStatus InstallOptionsWidget::currentStatus() const {
    if (!forceCheck->isChecked() && !asDepsCheck->isChecked()) return AlpmPackage::DO_INSTALL;
    else if (forceCheck->isChecked() && !asDepsCheck->isChecked()) return AlpmPackage::DO_INSTALL_FORCE;
    else if (!forceCheck->isChecked() && asDepsCheck->isChecked()) return AlpmPackage::DO_INSTALL_ASDEPS;
    return AlpmPackage::DO_INSTALL_ASDEPS_FORCE;
}

bool InstallOptionsWidget::setStatus(AlpmPackage::UserChangeStatus status) {
    if (status != AlpmPackage::DO_INSTALL &&
        status != AlpmPackage::DO_INSTALL_FORCE &&
        status != AlpmPackage::DO_INSTALL_ASDEPS &&
        status != AlpmPackage::DO_INSTALL_ASDEPS_FORCE) return false;
    forceCheck->setChecked((status & AlpmPackage::DO_INSTALL_FORCE) || (status & AlpmPackage::DO_INSTALL_ASDEPS_FORCE));
    asDepsCheck->setChecked((status & AlpmPackage::DO_INSTALL_ASDEPS) || (status & AlpmPackage::DO_INSTALL_ASDEPS_FORCE));
    return true;
}
