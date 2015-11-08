/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanlocalpackageslistdelegate.h"
#include "pacmansimpleitemmodel.h"
#include <QPainter>

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

PacmanLocalPackagesListDelegate::PacmanLocalPackagesListDelegate(QObject *parent) : QStyledItemDelegate(parent) {
    font = ((QWidget *)((QObject *)this)->parent())->font();
    bfont = font;
    bfont.setBold(true);
}

void PacmanLocalPackagesListDelegate::setModel(PacmanSimpleItemModel * _model) {
    model = _model;
}

QSize PacmanLocalPackagesListDelegate::sizeHint(const QStyleOptionViewItem & /*option*/,const QModelIndex & index) const {
    return model->data(index,Qt::SizeHintRole).toSize();
}

void PacmanLocalPackagesListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if ((option.state & QStyle::State_Selected) == QStyle::State_Selected) {
        painter->fillRect(option.rect,option.palette.highlight());
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    }
    else {
        if ((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver) {
            painter->fillRect(option.rect,option.palette.highlight().color().lighter());
            painter->setPen(option.palette.color(QPalette::HighlightedText));
        }
        else {
            painter->fillRect(option.rect,option.palette.color(QPalette::Base));
            painter->setPen(option.palette.color(QPalette::Text));
        }
    }

    painter->setFont(font);

    if(index.column() == 0) {
        painter->drawPixmap(0,option.rect.y(),model->data(index,Qt::DecorationRole).value<QIcon>().pixmap(22,22));
        painter->setFont(bfont);
        painter->drawText(option.rect.x()+option.decorationSize.width()+4,option.rect.y(),option.rect.width()-option.decorationSize.width(),option.rect.height(),Qt::AlignLeft|Qt::AlignVCenter,model->data(index,Qt::DisplayRole).toString());
        painter->setFont(font);
    }
    else painter->drawText(option.rect.x()+2,option.rect.y(),option.rect.width()-2,option.rect.height(),Qt::AlignLeft|Qt::AlignVCenter,model->data(index,Qt::DisplayRole).toString());
}
