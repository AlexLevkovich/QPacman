/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H

#include <QWidget>
#include <QIcon>
#include "alpmpackage.h"

class QEvent;
class QVBoxLayout;
class QCheckBox;

class OptionsWidget : public QWidget {
public:
    OptionsWidget(QWidget *parent = nullptr);
    virtual AlpmPackage::UserChangeStatus currentStatus() const = 0;
    virtual bool setStatus(AlpmPackage::UserChangeStatus status) = 0;
    QString currentStatusText() const;
    QString currentStatusTextHint() const;
    QIcon currentStatusIcon() const;
    static const QString statusText(AlpmPackage::UserChangeStatus status);
    static const QString statusTextHint(AlpmPackage::UserChangeStatus status);
    static const QIcon statusIcon(AlpmPackage::UserChangeStatus status);
    static uint maxTextWidth(const QFont & font);

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private:
    static void init_text();

    static QIcon install_icon;
    static QIcon reinstall_icon;
    static QIcon uninstall_icon;
    static QString DO_INSTALL_STR;
    static QString DO_INSTALL_ASDEPS_STR;
    static QString DO_INSTALL_ASDEPS_FORCE_STR;
    static QString DO_INSTALL_FORCE_STR;
    static QString DO_REINSTALL_STR;
    static QString DO_REINSTALL_ASDEPS_STR;
    static QString DO_UNINSTALL_STR;
    static QString DO_UNINSTALL_ALL_STR;
    static bool first_time_icon;
    static bool first_time_text;
};

class UninstallOptionsWidget : public OptionsWidget {
    Q_OBJECT
public:
    explicit UninstallOptionsWidget(QWidget *parent = nullptr);
    AlpmPackage::UserChangeStatus currentStatus() const;
    bool setStatus(AlpmPackage::UserChangeStatus status);

private:
    QVBoxLayout *verticalLayout;
    QCheckBox *depsCheck;
};

class ReinstallOptionsWidget : public OptionsWidget {
    Q_OBJECT
public:
    explicit ReinstallOptionsWidget(QWidget *parent = nullptr);
    AlpmPackage::UserChangeStatus currentStatus() const;
    bool setStatus(AlpmPackage::UserChangeStatus status);

private:
    QVBoxLayout *verticalLayout;
    QCheckBox *asDepsCheck;
};

class InstallOptionsWidget : public OptionsWidget {
    Q_OBJECT
public:
    explicit InstallOptionsWidget(QWidget *parent = nullptr);

    AlpmPackage::UserChangeStatus currentStatus() const;
    bool setStatus(AlpmPackage::UserChangeStatus status);
private:
    QVBoxLayout *verticalLayout;
    QCheckBox *asDepsCheck;
    QCheckBox *forceCheck;
};

#endif // OPTIONSWIDGET_H
