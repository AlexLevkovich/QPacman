/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmansheetdelegate.h"

PacmanSheetDelegate::PacmanSheetDelegate(QTreeView *view, QWidget *parent) : QItemDelegate(parent) {
    m_view = view;
}

void PacmanSheetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    if (model->hasChildren(index)) {
        // this is a top-level item.
        QStyleOptionButton buttonOption;
        buttonOption.state = option.state;
        buttonOption.state &= ~QStyle::State_HasFocus;
        buttonOption.rect = option.rect;
        buttonOption.palette = option.palette;
        buttonOption.features = QStyleOptionButton::None;
        buttonOption.fontMetrics = option.fontMetrics;
        buttonOption.iconSize = option.decorationSize;
        m_view->style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter, m_view);

        int i = 0;
        QRect r = option.rect;
        if (m_view->itemsExpandable()) {
            i = 9;
            QStyleOption branchOption;
            branchOption.rect = QRect(r.left() + i/2, r.top() + (r.height() - i)/2, i, i);
            branchOption.palette = option.palette;
            branchOption.state = QStyle::State_Children;

            if (m_view->isExpanded(index))
                branchOption.state |= QStyle::State_Open;

            m_view->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, m_view);
        }

        int decorationSize_width = option.decorationSize.width();
        QIcon icon = model->data(index, Qt::DecorationRole).value<QIcon>();
        if (!icon.isNull()) {
            QRect iconrect = QRect(r.left() + i*2, r.top(),decorationSize_width, r.height());
            m_view->style()->drawItemPixmap(painter,iconrect,Qt::AlignLeft,icon.pixmap(option.decorationSize));
        }
        else decorationSize_width = 0;

        // draw text
        QRect textrect = QRect(r.left() + i*2 + decorationSize_width + 2, r.top(), r.width() - ((5*i)/2) - decorationSize_width - 2, r.height());
        QString text = elidedText(option.fontMetrics, textrect.width(), Qt::ElideMiddle,model->data(index, Qt::DisplayRole).toString());
        m_view->style()->drawItemText(painter, textrect,Qt::AlignLeft|Qt::AlignVCenter, option.palette, m_view->isEnabled(), text);

    } else {
        QItemDelegate::paint(painter, option, index);
    }
}

QSize PacmanSheetDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const {
    QStyleOptionViewItem option = opt;
    return QItemDelegate::sizeHint(opt, index) + QSize(2, 5);
}
