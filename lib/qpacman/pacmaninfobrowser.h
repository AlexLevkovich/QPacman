/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanInfoBrowser_H
#define PacmanInfoBrowser_H

#include "textedithelper.h"
#include "alpmpackage.h"
#include "custompopuptextbrowser.h"

class AlpmPackage;

class PacmanInfoBrowser : public CustomPopupTextBrowser, public TextEditHelper {
    Q_OBJECT
public:
    explicit PacmanInfoBrowser(QWidget *parent = 0);
    void fillByInfo(const AlpmPackage & pkg,bool readonly = false);
    void refill();

protected:
    void setSource(const QUrl & name);

protected slots:
    void openMailUrl(const QUrl & name);
    void openUrl(const QUrl & name);

signals:
    void groupUrlSelected(const QString &);
    void packageUrlSelected(const QString & name,const QString & version,int operation);
    void reasonUrlSelected(const QString &);

private:
    void runProgram(const QString & name,const QStringList & args);
    int calculateFirstFolumnFidth(int count) const;
    static const QString modifyMailAddress(const QString & str);
    static bool isMailAddress(const QString & str);

    AlpmPackage m_pkg;
    QList<QString> fieldNames;
    friend class PacmanInfoBrowserDocument;
};

#endif // PacmanInfoBrowser_H
