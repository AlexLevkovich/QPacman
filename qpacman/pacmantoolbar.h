/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef QPACMANTOOLBAR_H
#define QPACMANTOOLBAR_H

#include <QToolBar>

class SearchWidget;

class PacmanToolBar : public QToolBar {
    Q_OBJECT
public:
    explicit PacmanToolBar(QWidget *parent = 0);
    SearchWidget * findSearchWidget();
};

#endif // QPACMANTOOLBAR_H
