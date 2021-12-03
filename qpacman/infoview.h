/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef INFOVIEW_H
#define INFOVIEW_H

#include "custompopuptextbrowser.h"
#include "textedithelper.h"

class QShowEvent;
class QTextTable;

class InfoView : public CustomPopupTextBrowser, public TextEditHelper {
    Q_OBJECT
public:
    InfoView(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event);

private:
    QTextTable * table;
};

#endif // INFOVIEW_H
