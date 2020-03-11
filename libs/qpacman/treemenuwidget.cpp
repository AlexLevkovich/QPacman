/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "treemenuwidget.h"
#include <QActionEvent>
#include <QAction>
#include <QMenu>
#include <QTreeWidgetItem>
#include <QDesktopWidget>
#include <QApplication>
#include <QHeaderView>
#include <QMouseEvent>
#include <QShowEvent>
#include "sheetdelegate.h"
#include "themeicons.h"

Q_DECLARE_METATYPE(QAction *)

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif

TreeMenuWidget::TreeMenuWidget(QWidget *parent) : QTreeWidget(parent) {
    connect(this,SIGNAL(triggered(QAction*)),parent,SIGNAL(triggered(QAction*)));
    connect(this,SIGNAL(hovered(QAction*)),parent,SIGNAL(hovered(QAction*)));

    setColumnCount(1);
    setHeaderHidden(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setMouseTracking(true);
    setItemDelegate(new SheetDelegate(this,parent));
    setItemsExpandable(false);
    setRootIsDecorated(false);

    QPalette p(palette());
    p.setColor(QPalette::Base,parent->palette().color(QPalette::AlternateBase));
    setPalette(p);

    setFrameShape(QFrame::NoFrame);
}

void TreeMenuWidget::insertAction(QAction * parent_action,QAction * before,QAction * action) {
    action->setData(QVariant::fromValue<QAction *>(parent_action));
    QTreeWidget::insertAction(before,action);
}

void TreeMenuWidget::actionEvent(QActionEvent * e) {
    switch (e->type()) {
        case QActionEvent::ActionAdded:
            addItem(e->action(),e->before());
            break;
        case QActionEvent::ActionRemoved:
            removeItem(e->action());
            break;
        case QActionEvent::ActionChanged:
            changeItem(e->action());
            break;
        default:
            QTreeWidget::actionEvent(e);
    }
}

void TreeMenuWidget::removeItem(QAction * action) {
    for (int i=0;i<model()->rowCount();i++) {
        QTreeWidgetItem * temp_item = itemFromIndex(model()->index(i,0));
        if (temp_item == NULL) continue;

        if (temp_item->data(0,Qt::UserRole).value<QAction *>() == action) {
            delete temp_item;
            return;
        }
    }
}

void TreeMenuWidget::mouseMoveEvent(QMouseEvent * event) {
    QTreeWidgetItem * item = itemAt(event->pos());
    if (item != NULL) {
        selectionModel()->select(indexFromItem(item,0),QItemSelectionModel::ClearAndSelect);
        setCurrentItem(item);
        emit hovered(item->data(0,Qt::UserRole).value<QAction *>());
    }
}

void TreeMenuWidget::mousePressEvent(QMouseEvent * event) {
    QTreeWidgetItem * item = itemAt(event->pos());
    if ((item != NULL) && (item->childCount() <= 0)) {
        ((QMenu *)parent())->hide();
        emit triggered(item->data(0,Qt::UserRole).value<QAction *>());
    }

    if ((item != NULL) && (item->childCount() > 0) && itemsExpandable()) {
        item->setExpanded(!item->isExpanded());
    }
}

void TreeMenuWidget::showEvent(QShowEvent * /*event*/) {
    for (int i=0;i<model()->rowCount();i++) {
        QTreeWidgetItem * temp_item = itemFromIndex(model()->index(i,0));
        if (temp_item->childCount() > 0) continue;
        temp_item->setSelected(true);
        setCurrentItem(temp_item);
    }

    QTreeWidgetItem * item = itemFromIndex(model()->index(0,0));
    if (item != NULL) scrollToItem(item);
}

void TreeMenuWidget::changeItem(QAction * action) {
    for (int i=0;i<model()->rowCount();i++) {
        QTreeWidgetItem * temp_item = itemFromIndex(model()->index(i,0));
        if (temp_item == NULL) continue;

        if (temp_item->data(0,Qt::UserRole).value<QAction *>() == action) {
            setItem(temp_item,action);
            return;
        }
    }
}

QTreeWidgetItem * TreeMenuWidget::initItem(QTreeWidgetItem * parent,TreeMenuWidget * widget,QAction * action) {
    QTreeWidgetItem * temp_item = NULL;
    if (parent != NULL) temp_item = new QTreeWidgetItem(parent);
    else if (widget != NULL) temp_item = new QTreeWidgetItem(this);
    else temp_item = new QTreeWidgetItem();

    setItem(temp_item,action);

    return temp_item;
}

void TreeMenuWidget::setItem(QTreeWidgetItem * item,QAction * action) {
    item->setData(0,Qt::UserRole,QVariant::fromValue<QAction *>(action));
    item->setText(0,action->iconText());
    QIcon icon = action->icon();
    item->setIcon(0,action->isIconVisibleInMenu()?((icon.isNull())?ThemeIcons::get(ThemeIcons::NO_ICON):icon):QIcon());
    if (action->isCheckable()) item->setCheckState(0,action->isChecked()?Qt::Checked:Qt::Unchecked);
    item->setFont(0,action->font());
}

void TreeMenuWidget::addItem(QAction * action,QAction * before) {
    if (action->inherits("QWidgetAction")) return;
    if (action->isSeparator()) return;

    QTreeWidgetItem * temp_item = NULL;
    QAction * menuAction = action->data().value<QAction *>();
    if (menuAction != NULL) {
        QTreeWidgetItem * item = NULL;
        for (int i=0;i<model()->rowCount();i++) {
            temp_item = itemFromIndex(model()->index(i,0));
            if (temp_item == NULL) continue;

            if (temp_item->data(0,Qt::UserRole).value<QAction *>() == menuAction) {
                item = temp_item;
                break;
            }
        }

        temp_item = NULL;
        if (item != NULL) temp_item = addSimpleItem(item,action,before);
    }
    else temp_item = addSimpleItem(action,before);
    if (temp_item == NULL) return;

    if (action->menu() != NULL) addMenuItem(temp_item,action);
}

void TreeMenuWidget::addMenuItem(QTreeWidgetItem * item,QAction * action) {
    QList<QAction *> actions = action->menu()->actions();
    for (int i=0;i<actions.count();i++) {
        QTreeWidgetItem * temp_item = initItem(item,NULL,actions[i]);
        if (actions[i]->menu() != NULL) addMenuItem(temp_item,actions[i]);
    }
}

QTreeWidgetItem * TreeMenuWidget::addSimpleItem(QAction * action,QAction * before) {
    if (before == NULL) {
        return initItem(NULL,this,action);
    }

    for (int i=0;i<topLevelItemCount();i++) {
        QTreeWidgetItem * root_item = topLevelItem(i);
        if (root_item->data(0,Qt::UserRole).value<QAction *>() == before) {
            QTreeWidgetItem * temp_item = initItem(NULL,NULL,action);
            insertTopLevelItem(i,temp_item);
            return temp_item;
        }

        root_item = addSimpleChildItem(root_item,action,before);
        if (root_item != NULL) return root_item;
    }

    return NULL;
}

QTreeWidgetItem * TreeMenuWidget::addSimpleItem(QTreeWidgetItem * parent_item,QAction * action,QAction * before) {
    if (before == NULL) {
        return initItem(parent_item,NULL,action);
    }

    return addSimpleChildItem(parent_item,action,before);
}

QTreeWidgetItem * TreeMenuWidget::addSimpleChildItem(QTreeWidgetItem * item,QAction * action,QAction * before) {
    for (int i=0;i<item->childCount();i++) {
        QTreeWidgetItem * child_item = item->child(i);
        if (child_item->data(0,Qt::UserRole).value<QAction *>() == before) {
            QTreeWidgetItem * temp_item = initItem(NULL,NULL,action);
            item->insertChild(i,temp_item);
            return temp_item;
        }
        child_item = addSimpleChildItem(child_item,action,before);
        if (child_item != NULL) return child_item;
    }

    return NULL;
}

void TreeMenuWidget::calculateHeight() {
    int h = 0;

    int topLevelCount = topLevelItemCount();

    for(int i = 0;i < topLevelCount;i++) {
        QTreeWidgetItem * item = topLevelItem(i);
        h += calculateHeightRec(item);
    }

    if(h != 0) {
        h += header()->sizeHint().height();
        setMinimumHeight(min(h,QApplication::desktop()->availableGeometry(this).height()-10));
    }
}

int TreeMenuWidget::calculateHeightRec(QTreeWidgetItem * item) {
    if(!item)
        return 0;

    QModelIndex index = indexFromItem(item);

    if(!item->isExpanded()) {
        return rowHeight(index);
    }

    int h = rowHeight(index);
    int childCount = item->childCount();
    for(int i = 0; i < childCount;i++) {
        h += calculateHeightRec(item->child(i));
    }

    return h;
}

