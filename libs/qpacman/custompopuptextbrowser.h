/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef CUSTOMPOPUPTEXTBROWSER_H
#define CUSTOMPOPUPTEXTBROWSER_H

#include <QTextBrowser>

class QKeyEvent;

class CustomPopupTextBrowser : public QTextBrowser {
    Q_OBJECT
public:
    CustomPopupTextBrowser(QWidget *parent = nullptr);

private slots:
    void showContextMenu(const QPoint &pt);
    void copy_selected();

protected:
    void keyPressEvent(QKeyEvent *e);

};

#endif // CUSTOMPOPUPTEXTBROWSER_H
