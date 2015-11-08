/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANLOCALPACKAGESLISTDELEGATE_H
#define PACMANLOCALPACKAGESLISTDELEGATE_H

#include <QStyledItemDelegate>

class PacmanSimpleItemModel;

class PacmanLocalPackagesListDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit PacmanLocalPackagesListDelegate(QObject *parent = 0);
    void setModel(PacmanSimpleItemModel * _model);
    QSize sizeHint(const QStyleOptionViewItem & option,const QModelIndex & index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    PacmanSimpleItemModel * model;
    QFont font;
    QFont bfont;
};

#endif // PACMANLOCALPACKAGESLISTDELEGATE_H
