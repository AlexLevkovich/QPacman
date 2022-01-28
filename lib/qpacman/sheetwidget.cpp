/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "sheetwidget.h"
#include "sheetdelegate.h"
#include <QMouseEvent>
#include <QHeaderView>
#include <QScrollBar>
#include <QDebug>
#include "static.h"

SheetWidget::SheetWidget(QWidget *parent) : QTreeWidget(parent) {
    setItemDelegate(new SheetDelegate(this,parent));
    setItemsExpandable(true);
    setRootIsDecorated(false);
    setWordWrap(false);


    setIconSize(quadroSize(fontMetrics().height()+4));

    connect(this,&SheetWidget::itemCollapsed,this,&SheetWidget::collapseExpandItem);
    connect(this,&SheetWidget::itemExpanded,this,&SheetWidget::collapseExpandItem);
    connect(header(),&QHeaderView::sectionCountChanged,this,&SheetWidget::onHeaderSectionCountChanged);
    connect(this,&SheetWidget::itemChanged,this,&SheetWidget::onItemChanged);
}

void SheetWidget::onItemChanged(QTreeWidgetItem * item,int col) {
    if (item->parent() != NULL && item->parent()->parent() != NULL) return;
    if (item->parent() != NULL || (item->childCount() <= 0 && col > 0)) {
        if (item->parent() != NULL) item->parent()->setFirstColumnSpanned(true);
        column_widths[col] = qMax(qMax(column_widths[col],header()->iconSize().width()+20+header()->fontMetrics().horizontalAdvance(model()->headerData(col,Qt::Horizontal).toString())+(frameWidth()*2)),fontMetrics().horizontalAdvance(item->text(col))+(frameWidth()*2)+style()->pixelMetric(QStyle::PM_TreeViewIndentation,0,this)+(2*style()->pixelMetric(QStyle::PM_HeaderMargin,0,this))+iconSize().width());
        header()->resizeSection(col,column_widths[col]);

        if (item->parent() == NULL && item->childCount() <= 0 && col == 1) {
            column_widths[0] = qMax(qMax(column_widths[0],header()->iconSize().width()+20+header()->fontMetrics().horizontalAdvance(model()->headerData(0,Qt::Horizontal).toString())+(frameWidth()*2)),fontMetrics().horizontalAdvance(item->text(0))+(frameWidth()*2)+style()->pixelMetric(QStyle::PM_TreeViewIndentation,0,this)+(2*style()->pixelMetric(QStyle::PM_HeaderMargin,0,this))+iconSize().width());
            header()->resizeSection(0,column_widths[0]);
        }
    }
    int root_item_width = iconSize().width()+20+style()->pixelMetric(QStyle::PM_TreeViewIndentation,0,this)+fontMetrics().horizontalAdvance((item->parent() == NULL)?item->text(0):item->parent()->text(0))+(frameWidth()*2);
    int header_width = header()->length()+(frameWidth()*2);
    if (root_item_width > header_width) header()->resizeSection(0,header()->sectionSize(0)+(root_item_width - header_width));
    setMinimumWidth(qMax(header_width,root_item_width)+(verticalScrollBar()->isVisible()?verticalScrollBar()->width():0));
}

void SheetWidget::onHeaderSectionCountChanged(int old_count,int new_count) {
    if (old_count < new_count && column_widths.count() < new_count) {
        for (int i=((old_count > column_widths.count())?0:old_count);i<new_count;i++) {
            header()->setSectionResizeMode(i,QHeaderView::Interactive);
            column_widths.append(1);
        }
    }
    else if (old_count > new_count) {
        for (int i=(old_count-1);i>=new_count;i--) {
            column_widths.remove(i);
        }
    }
}

void SheetWidget::mousePressEvent(QMouseEvent * event) {
    QTreeWidget::mousePressEvent(event);

    QTreeWidgetItem * item = itemAt(event->pos());
    if ((item != NULL) && (item->childCount() > 0) && itemsExpandable()) {
        item->setExpanded(!item->isExpanded());
    }
}

void SheetWidget::collapseExpandItem() {
    header()->setVisible(existsExpandedRootItem());
}

bool SheetWidget::existsExpandedRootItem() {
    for (int i=0;i<topLevelItemCount();i++) {
        QTreeWidgetItem * item = topLevelItem(i);
        if ((item->childCount() > 0) && item->isExpanded()) return true;
    }

    return false;
}

void SheetWidget::showEvent(QShowEvent *event) {
    QTreeWidget::showEvent(event);
    setMinimumWidth(minimumWidth()+(verticalScrollBar()->isVisible()?verticalScrollBar()->width():0));
}
