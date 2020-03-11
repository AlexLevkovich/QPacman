/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef SHEETWIDGET_H
#define SHEETWIDGET_H

#include <QTreeWidget>
#include <QVector>

class SheetWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit SheetWidget(QWidget *parent = 0);

private slots:
    void collapseExpandItem();

protected:
    void mousePressEvent(QMouseEvent * event);
    void drawBranches(QPainter*,const QRect&,const QModelIndex&) const {;}
    void showEvent(QShowEvent *event);

private slots:
    void headerSectionCountChanged(int old_count,int new_count);
    void itemChanged(QTreeWidgetItem * item,int col);

private:
    bool existsExpandedRootItem();

    QVector<int> column_widths;
};

#endif // SheetWidget_H
