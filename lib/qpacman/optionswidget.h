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
    static uint maxTextWidth(const QFont & font);

protected:
    bool eventFilter(QObject *obj, QEvent *e);
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
