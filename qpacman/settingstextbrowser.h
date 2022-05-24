/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/

#ifndef SETTINGSTEXTBROWSER_H
#define SETTINGSTEXTBROWSER_H

#include <custompopuptextbrowser.h>
#include "widgettextobject.h"
#include "textedithelper.h"
#include <QLineEdit>
#include <QNetworkProxy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QIcon>
#include <QBrush>
#include <QStyleOptionViewItem>
#include <QFont>
#include <QStyledItemDelegate>
#include "libalpm.h"

class QSpinBox;
class QLayout;
class QComboBox;
class QHideEvent;
class QShowEvent;
class QTextTable;
class QRadioButton;
class QDialogButtonBox;
class QFrame;
class QLabel;
class QListView;
class QLineEdit;
class RepoItemModel;
class QButtonGroup;
class QTextTable;

class RepoSettingsTextObject : public WidgetTextObject {
    Q_OBJECT
public:
    RepoSettingsTextObject(QTextEdit * edit,const Alpm::Repo & repo);
    RepoSettingsTextObject(QTextEdit * edit);

    Alpm::Repo repo(bool & usemirror) const;

signals:
    void result(bool ok);

private slots:
    void package_siglevel_update_states();
    void database_siglevel_update_states();
    void servers_update_states();
    void usage_update_states();
    void ok_button_state();
    void selection_repolist_state();
    void add_repolist_item();
    void delete_repolist_item();
    void apply_repo();

private:
    Alpm::Repo::SigCheck sigcheck(bool package) const;
    Alpm::Repo::SigAllowed sigallowed(bool package) const;
    void init();
    void update_states();
    bool is_servers_set() const;
    bool at_least_one_arch_picked() const;
    QStringList checked_arches() const;

    QLabel *label;
    QLineEdit *nameEdit;
    QRadioButton *mirrorRadio;
    QRadioButton *serversRadio;
    QListView *serversList;
    QToolButton *addButton;
    QToolButton *delButton;
    QFrame *line;
    QCheckBox *packageCheck;
    QRadioButton *neverPackageRadio;
    QRadioButton *optionalPackageRadio;
    QRadioButton *requiredPackageRadio;
    QRadioButton *trustedonlyPackageRadio;
    QRadioButton *trustallPackageRadio;
    QCheckBox *databaseCheck;
    QRadioButton *neverDatabaseRadio;
    QRadioButton *optionalDatabaseRadio;
    QRadioButton *requiredDatabaseRadio;
    QRadioButton *trustedonlyDatabaseRadio;
    QRadioButton *trustallDatabaseRadio;
    QCheckBox *syncCheck;
    QCheckBox *searchCheck;
    QCheckBox *installCheck;
    QCheckBox *upgradeCheck;
    QCheckBox *allCheck;
    QDialogButtonBox *buttonBox;
    QButtonGroup * buttonGroup_2;
    QButtonGroup * buttonGroup_3;
    QButtonGroup * buttonGroup_4;
    QButtonGroup * buttonGroup_5;
    QButtonGroup * buttonGroup_6;
    Alpm::Repo m_repo;
};

class LabelSpinTextObject : public WidgetTextObject {
    Q_OBJECT
public:
    LabelSpinTextObject(QTextEdit * edit,const QString & text,int min,int max,int value,const QString & suffix = QString());
    LabelSpinTextObject(QTextEdit * edit,const QString & text,int min,int max,const QString & suffix = QString());
    int value() const;
    void setValue(int value);
    bool isEnabled() const;
    void setEnabled(bool flag);
    QString text() const;
private:
    void init(const QString & text,int min,int max,int value,const QString & suffix);

    QSpinBox * spin;
    QLabel * label;
};

class LabelLineEditTextObject : public WidgetTextObject {
    Q_OBJECT
public:
    LabelLineEditTextObject(QTextEdit * edit,const QString & labelText,const QString & text = QString());
    void setEchoMode(QLineEdit::EchoMode mode);
    QString text() const;
    QString lineText() const;
    bool isEnabled() const;
    void setEnabled(bool flag);
    void setLineText(const QString & text);
private:
    QLineEdit * lineEdit;
    QLabel * label;
};

class ProxyTypeObject : public WidgetTextObject {
    Q_OBJECT
public:
    ProxyTypeObject(QTextEdit * edit,const QString & text);
    QNetworkProxy::ProxyType value() const;
    QString text() const;
    void setValue(QNetworkProxy::ProxyType type);
private slots:
    void proxyActivated(int index);
signals:
    void activated(int type);
private:
    QComboBox * combo;
    QLabel * label;
};

class DBExtObject : public WidgetTextObject {
    Q_OBJECT
public:
    DBExtObject(QTextEdit * edit,const QString & text);
    QString value() const;
    QString text() const;
    void comboRefresh();
private slots:
    void extActivated(int index);
signals:
    void activated(const QString & ext);
private:
    QComboBox * combo;
    QLabel * label;
};

class SettingsTextBrowser : public CustomPopupTextBrowser, public TextEditHelper {
    Q_OBJECT
public:
    SettingsTextBrowser(QWidget *parent = nullptr);

private slots:
    void proxyActivated(int index);
    void addNewSettingsResult(bool result);
    void changeSettingsResult(bool result);
    void addnew_button_clicked();
    void resize_addnew_button(QTextTable * table,ButtonTextObject * button);

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

signals:
    void dbsRefreshNeeded();

private:
    void recreate_repo_table();
    void change_button_clicked(const QString & repo);
    void remove_button_clicked(const QString & repo);
    Alpm::Repo repo(const QString & name) const;

    CheckBoxTextObject * useSysIconsBox;
    ProxyTypeObject * proxyType;
    DBExtObject * extObj;
    LabelLineEditTextObject * proxyAddress;
    LabelLineEditTextObject * proxyUser;
    LabelLineEditTextObject * proxyPassword;
    LabelSpinTextObject * proxyPort;
    LabelSpinTextObject * threadsObj;
    LabelSpinTextObject * timeoutObj;
    int repoStartPos;
    bool emit_refresh_at_hide;
};

#endif // SETTINGSTEXTBROWSER_H
