/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef TREEITEMSMENU_H
#define TREEITEMSMENU_H

#include <QMenu>

class QActionEvent;
class TreeMenuWidget;
class QWidgetAction;

class TreeItemsMenu : public QMenu {
    Q_OBJECT
public:
    TreeItemsMenu(QWidget *parent = 0);

protected:
    void actionEvent(QActionEvent * e);
    bool eventFilter(QObject *obj,QEvent *event);

private slots:
    void aboutToShow();

private:
    QWidgetAction * treeAction;
    TreeMenuWidget * treeWidget;
    bool isFirstTime;
};

#endif // TREEITEMSMENU_H
