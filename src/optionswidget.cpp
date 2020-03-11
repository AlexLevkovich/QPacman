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

QIcon OptionsWidget::install_icon = QIcon();
QIcon OptionsWidget::reinstall_icon = QIcon();
QIcon OptionsWidget::uninstall_icon = QIcon();

QString OptionsWidget::DO_INSTALL_STR = QString();
QString OptionsWidget::DO_INSTALL_ASDEPS_STR = QString();
QString OptionsWidget::DO_INSTALL_ASDEPS_FORCE_STR = QString();
QString OptionsWidget::DO_INSTALL_FORCE_STR = QString();
QString OptionsWidget::DO_REINSTALL_STR = QString();
QString OptionsWidget::DO_REINSTALL_ASDEPS_STR = QString();
QString OptionsWidget::DO_UNINSTALL_STR = QString();
QString OptionsWidget::DO_UNINSTALL_ALL_STR = QString();

bool OptionsWidget::first_time_icon = true;
bool OptionsWidget::first_time_text = true;

OptionsWidget::OptionsWidget(QWidget *parent) : QWidget(parent) {
    qApp->installEventFilter(this);
}

const QString OptionsWidget::statusTextHint(AlpmPackage::UserChangeStatus status) {
    init_text();

    switch (status) {
    case AlpmPackage::DO_INSTALL:
        return DO_INSTALL_STR;
    case AlpmPackage::DO_INSTALL_ASDEPS:
        return DO_INSTALL_ASDEPS_STR;
    case AlpmPackage::DO_INSTALL_ASDEPS_FORCE:
        return DO_INSTALL_ASDEPS_FORCE_STR;
    case AlpmPackage::DO_INSTALL_FORCE:
        return DO_INSTALL_FORCE_STR;
    case AlpmPackage::DO_REINSTALL:
        return DO_REINSTALL_STR;
    case AlpmPackage::DO_REINSTALL_ASDEPS:
        return DO_REINSTALL_ASDEPS_STR;
    case AlpmPackage::DO_UNINSTALL:
        return DO_UNINSTALL_STR;
    case AlpmPackage::DO_UNINSTALL_ALL:
        return DO_UNINSTALL_ALL_STR;
    default:
        return QString();
    }
}

const QString OptionsWidget::statusText(AlpmPackage::UserChangeStatus status) {
    init_text();

    switch (status) {
    case AlpmPackage::DO_INSTALL:
    case AlpmPackage::DO_INSTALL_ASDEPS:
    case AlpmPackage::DO_INSTALL_ASDEPS_FORCE:
    case AlpmPackage::DO_INSTALL_FORCE:
        return DO_INSTALL_STR;
    case AlpmPackage::DO_REINSTALL:
    case AlpmPackage::DO_REINSTALL_ASDEPS:
        return DO_REINSTALL_STR;
    case AlpmPackage::DO_UNINSTALL:
    case AlpmPackage::DO_UNINSTALL_ALL:
        return DO_UNINSTALL_STR;
    default:
        return QString();
    }
}

void OptionsWidget::init_text() {
    if (first_time_text) {
        first_time_text = false;
        DO_INSTALL_STR = QObject::tr("Install");
        DO_INSTALL_ASDEPS_STR = QObject::tr("Install as a dependency");
        DO_INSTALL_ASDEPS_FORCE_STR = QObject::tr("Install forcibly as a dependency");
        DO_INSTALL_FORCE_STR = QObject::tr("Install forcibly");
        DO_REINSTALL_STR = QObject::tr("Reinstall");
        DO_REINSTALL_ASDEPS_STR = QObject::tr("Reinstall as a dependency");
        DO_UNINSTALL_STR = QObject::tr("Uninstall");
        DO_UNINSTALL_ALL_STR = QObject::tr("Uninstall with dependencies");
    }
}

uint OptionsWidget::maxTextWidth(const QFont & font) {
    init_text();

    QFontMetrics fm(font);
    return (uint)qMax(fm.horizontalAdvance(DO_INSTALL_STR),qMax(fm.horizontalAdvance(DO_REINSTALL_STR),fm.horizontalAdvance(DO_UNINSTALL_STR)));
}

QString OptionsWidget::currentStatusText() const {
    return statusText(currentStatus());
}

QString OptionsWidget::currentStatusTextHint() const {
    return statusTextHint(currentStatus());
}

const QIcon OptionsWidget::statusIcon(AlpmPackage::UserChangeStatus status) {
    if (first_time_icon) {
        first_time_icon = false;
        install_icon = ThemeIcons::get(ThemeIcons::PKG_INSTALL_MARK);
        reinstall_icon = ThemeIcons::get(ThemeIcons::PKG_REINSTALL_MARK);
        uninstall_icon = ThemeIcons::get(ThemeIcons::PKG_REMOVE_MARK);
    }

    switch (status) {
    case AlpmPackage::DO_INSTALL:
    case AlpmPackage::DO_INSTALL_ASDEPS:
    case AlpmPackage::DO_INSTALL_ASDEPS_FORCE:
    case AlpmPackage::DO_INSTALL_FORCE:
        return install_icon;
    case AlpmPackage::DO_REINSTALL:
    case AlpmPackage::DO_REINSTALL_ASDEPS:
        return reinstall_icon;
    case AlpmPackage::DO_UNINSTALL:
    case AlpmPackage::DO_UNINSTALL_ALL:
        return uninstall_icon;
    default:
        return QIcon();
    }
}

QIcon OptionsWidget::currentStatusIcon() const {
    return statusIcon(currentStatus());
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
