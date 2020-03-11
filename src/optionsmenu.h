/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#include <QMenu>
#include <QVector>
#include "alpmpackage.h"

class OptionsWidget;
class InstallOptionsWidget;
class ReinstallOptionsWidget;
class UninstallOptionsWidget;
class QHBoxLayout;
class QLabel;
class QSpacerItem;
class QRadioButton;
class QToolButton;

class OkCancelTitleBar : public QWidget {
    Q_OBJECT
public:
    OkCancelTitleBar(int index,QWidget *parent = nullptr);
    bool isRadioButtonVisible() const;
    void setRadioButtonVisible(bool visible);
    QString text() const;
    void setText(const QString & text);
    bool isOkButtonVisible() const;
    void setOkButtonVisible(bool visible);
    bool isCancelButtonVisible() const;
    void setCancelButtonVisible(bool visible);
    bool isChecked() const;
    void setChecked(bool checked);
    void setIcon(const QIcon & icon);
    int index() const { return m_index; }

signals:
    void checkStateChanged(bool checked);
    void okClicked();
    void cancelClicked();

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private:
    int m_index;
    bool m_radio_visible;
    QHBoxLayout *horizontalLayout;
    QLabel *iconLabel;
    QSpacerItem *horizontalSpacer_2;
    QRadioButton *radioButton;
    QLabel *windowText;
    QSpacerItem *horizontalSpacer;
    QToolButton *cancelButton;
    QToolButton *okButton;

    friend class OptionsMenu;
};

class OptionsMenu : public QMenu {
    Q_OBJECT
public:
    OptionsMenu(AlpmPackage::UserChangeStatus status,QWidget *parent = nullptr);

private slots:
    void init();
    void show_event();
    void okClicked();
    void titleCheckStateChanged(bool state);

protected:
    bool eventFilter(QObject *obj, QEvent *e);
    virtual bool hasTitleOkButton() const;
    virtual bool hasTitleCancelButton() const;
    virtual bool haveTitleRadios() const = 0;
    virtual QStringList titleTexts() const = 0;
    virtual QList<OptionsWidget *> widgets() const = 0;

private:
    QList<OkCancelTitleBar *> titleBars() const;
    int checked_index;
    AlpmPackage::UserChangeStatus m_status;
    QVector<QWidget *> m_child_widgets;
    bool m_init_ok;
    bool m_show_event_ok;
};

class InstallOptionsMenu : public OptionsMenu {
    Q_OBJECT
public:
    InstallOptionsMenu(AlpmPackage::UserChangeStatus status,QWidget *parent = nullptr);

protected:
    QStringList titleTexts() const;
    bool haveTitleRadios() const;
    QList<OptionsWidget *> widgets() const;

private:
    InstallOptionsWidget * m_widget;
};


class ReinstallOptionsMenu : public OptionsMenu {
    Q_OBJECT
public:
    ReinstallOptionsMenu(AlpmPackage::UserChangeStatus status,QWidget *parent = nullptr);

protected:
    QStringList titleTexts() const;
    bool haveTitleRadios() const;
    QList<OptionsWidget *> widgets() const;

private:
    ReinstallOptionsWidget * m_rewidget;
    UninstallOptionsWidget * m_unwidget;
};

class UninstallOptionsMenu : public OptionsMenu {
    Q_OBJECT
public:
    UninstallOptionsMenu(AlpmPackage::UserChangeStatus status,QWidget *parent = nullptr);

protected:
    QStringList titleTexts() const;
    bool haveTitleRadios() const;
    QList<OptionsWidget *> widgets() const;

private:
    UninstallOptionsWidget * m_widget;
};

#endif // OPTIONSMENU_H
