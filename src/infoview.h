/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef INFOVIEW_H
#define INFOVIEW_H

#include "custompopuptextbrowser.h"
#include "textedithelper.h"
#include "widgettextobject.h"
#include <QNetworkProxy>
#include <QLineEdit>

class QSpinBox;
class QLayout;
class QComboBox;
class QHideEvent;

class LabelSpinTextObject : public WidgetTextObject {
    Q_OBJECT
public:
    LabelSpinTextObject(QTextEdit * edit,const QString & text,int min,int max,int value,const QString & suffix = QString());
    int value() const;
    bool isEnabled() const;
    void setEnabled(bool flag);
    QString text() const;
private:
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
private:
    QLineEdit * lineEdit;
    QLabel * label;
};

class ProxyTypeObject : public WidgetTextObject {
    Q_OBJECT
public:
    ProxyTypeObject(QTextEdit * edit,const QString & text,const QNetworkProxy & proxy);
    QNetworkProxy::ProxyType value() const;
    QString text() const;
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
private slots:
    void extActivated(int index);
signals:
    void activated(const QString & ext);
private:
    QComboBox * combo;
    QLabel * label;
};

class InfoView : public CustomPopupTextBrowser, public TextEditHelper {
    Q_OBJECT
public:
    InfoView(QWidget *parent = nullptr);

private slots:
    void proxyActivated(int index);

protected:
    void hideEvent(QHideEvent *event);

private:
    CheckBoxTextObject * useSysIconsBox;
    ProxyTypeObject * proxyType;
    DBExtObject * extObj;
    LabelLineEditTextObject * proxyAddress;
    LabelLineEditTextObject * proxyUser;
    LabelLineEditTextObject * proxyPassword;
    LabelSpinTextObject * proxyPort;

    LabelSpinTextObject * threadsObj;
    LabelSpinTextObject * timeoutObj;
};

#endif // INFOVIEW_H
