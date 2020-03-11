/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef TREEMENUWIDGET_H
#define TREEMENUWIDGET_H

#include <QTreeWidget>

class QActionEvent;
class QAction;
class QTreeWidgetItem;
class QMouseEvent;
class QShowEvent;

class TreeMenuWidget : public QTreeWidget {
    friend class TreeItemsMenu;

    Q_OBJECT
public:
    explicit TreeMenuWidget(QWidget *parent = 0);

protected:
    void actionEvent(QActionEvent * e);
    void mouseMoveEvent(QMouseEvent * event);
    void showEvent(QShowEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void drawBranches(QPainter*,const QRect&,const QModelIndex&) const {}

signals:
    void hovered(QAction * action);
    void triggered(QAction * action);

private:
    void insertAction(QAction * parent_action,QAction * before,QAction * action);
    QTreeWidgetItem * initItem(QTreeWidgetItem * parent,TreeMenuWidget * widget,QAction * action);
    void setItem(QTreeWidgetItem * item,QAction * action);
    void removeItem(QAction * action);
    void changeItem(QAction * action);
    void addItem(QAction * action,QAction * before);
    void addMenuItem(QTreeWidgetItem * item,QAction * action);
    QTreeWidgetItem * addSimpleItem(QAction * action,QAction * before);
    QTreeWidgetItem * addSimpleItem(QTreeWidgetItem * parent_item,QAction * action,QAction * before);
    QTreeWidgetItem * addSimpleChildItem(QTreeWidgetItem * item,QAction * action,QAction * before);
    void calculateHeight();
    int calculateHeightRec(QTreeWidgetItem * item);
};

#endif // TREEMENUWIDGET_H
