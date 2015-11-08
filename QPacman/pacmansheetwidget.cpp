/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmansheetwidget.h"
#include "pacmansheetdelegate.h"
#include <QMouseEvent>
#include <QHeaderView>

PacmanSheetWidget::PacmanSheetWidget(QWidget *parent) : QTreeWidget(parent) {
    setItemDelegate(new PacmanSheetDelegate(this,parent));
    setItemsExpandable(true);
    setRootIsDecorated(false);

    connect(this,SIGNAL(itemCollapsed(QTreeWidgetItem *)),this,SLOT(collapseExpandItem()));
    connect(this,SIGNAL(itemExpanded(QTreeWidgetItem *)),this,SLOT(collapseExpandItem()));
}

void PacmanSheetWidget::rowsInserted(const QModelIndex & parent,int start,int end) {
    QTreeWidget::rowsInserted(parent,start,end);
    if (!parent.isValid()) {
        for (int i=start;i<=end;i++) {
            QTreeWidgetItem * item = itemFromIndex(model()->index(i,0));
            item->setFirstColumnSpanned(true);
        }
    }
}

void PacmanSheetWidget::mousePressEvent(QMouseEvent * event) {
    QTreeWidget::mousePressEvent(event);

    QTreeWidgetItem * item = itemAt(event->pos());
    if ((item != NULL) && (item->childCount() > 0) && itemsExpandable()) {
        item->setExpanded(!item->isExpanded());
    }
}

void PacmanSheetWidget::collapseExpandItem() {
    header()->setVisible(existsExpandedRootItem());
}

bool PacmanSheetWidget::existsExpandedRootItem() {
    for (int i=0;i<topLevelItemCount();i++) {
        QTreeWidgetItem * item = topLevelItem(i);
        if ((item->childCount() > 0) && item->isExpanded()) return true;
    }

    return false;
}
