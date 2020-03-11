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
    void fillByInfo(AlpmPackage * pkg);

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
    static int calculateFirstFolumnFidth(int count);
    static const QString modifyMailAddress(const QString & str);
    static bool isMailAddress(const QString & str);

    static const QString fieldNames[20];
    friend class PacmanInfoBrowserDocument;
};

#endif // PacmanInfoBrowser_H
