/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef SHEETDELEGATE_H
#define SHEETDELEGATE_H

#include <QItemDelegate>
#include <QTreeView>

class SheetDelegate : public QItemDelegate {
    Q_OBJECT
public:
    explicit SheetDelegate(QTreeView *view, QWidget *parent);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;

private:
    QTreeView *m_view;
};

#endif // SheetDelegate_H
