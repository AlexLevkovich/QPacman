/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANSHEETDELEGATE_H
#define PACMANSHEETDELEGATE_H

#include <QItemDelegate>
#include <QTreeView>

class PacmanSheetDelegate : public QItemDelegate {
    Q_OBJECT
public:
    explicit PacmanSheetDelegate(QTreeView *view, QWidget *parent);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;

private:
    QTreeView *m_view;
};

#endif // PACMANSHEETDELEGATE_H
