/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANSHEETWIDGET_H
#define PACMANSHEETWIDGET_H

#include <QTreeWidget>
#include "libqpacman_global.h"

class LIBQPACMANSHARED_EXPORT PacmanSheetWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit PacmanSheetWidget(QWidget *parent = 0);

private slots:
    void collapseExpandItem();

protected:
    void rowsInserted(const QModelIndex & parent,int start,int end);
    void mousePressEvent(QMouseEvent * event);
    void drawBranches(QPainter*,const QRect&,const QModelIndex&) const {;};

private:
    bool existsExpandedRootItem();
};

#endif // PACMANSHEETWIDGET_H
