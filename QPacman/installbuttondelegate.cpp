/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "installbuttondelegate.h"
#include "pacmansimpleitemmodel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QMenu>
#include <QApplication>
#include "pacmanview.h"
#include <math.h>

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

int InstallButtonDelegate::lastColumn = 4;

InstallButtonDelegate::InstallButtonDelegate(QObject *parent) : QStyledItemDelegate(parent) {
    isArrowPressed = false;
    font = ((QWidget *)((QObject *)this)->parent())->font();
    bfont = font;
    bfont.setBold(true);
}

void InstallButtonDelegate::setModel(PacmanItemModel * _model) {
    model = _model;
}

QSize InstallButtonDelegate::sizeHint(const QStyleOptionViewItem & option,const QModelIndex & index) const {
    if (!index.isValid()) return QSize(0,0);

    if (index.column() < lastColumn) {
        return model->data(index,Qt::SizeHintRole).toSize();
    }
    else {
        QVariant display_data = model->data(index,Qt::DisplayRole);
        if (!display_data.isValid()) return QSize(0,0);
        int height = max(option.fontMetrics.height(),option.decorationSize.height()) - QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth)*2;
        int width = option.fontMetrics.width(display_data.toString()) + option.fontMetrics.width(' ')*2;
        width += option.decorationSize.width();
        width += QApplication::style()->pixelMetric(QStyle::PM_MenuButtonIndicator);
        return QApplication::style()->sizeFromContents(QStyle::CT_ToolButton,&option,QSize(width,height));
    }
}

class MenuItemData {
public:
    MenuItemData() {}
    MenuItemData(const QModelIndex &index,PacmanEntry::UserChangeStatus status,PacmanItemModel * model) {
        this->index = index;
        this->status = status;
        this->model = model;
    }

    QModelIndex index;
    PacmanEntry::UserChangeStatus status;
    PacmanItemModel * model;
};

Q_DECLARE_METATYPE(MenuItemData)

bool InstallButtonDelegate::editorEvent(QEvent *event,QAbstractItemModel *_model,const QStyleOptionViewItem &option,const QModelIndex &index) {
     if(((event->type() == QEvent::MouseButtonRelease) && (index.column() == lastColumn)) || (event->type() == QEvent::KeyPress)) {
         QRect arrowRect = option.rect;
         arrowRect.setX(arrowRect.x()+arrowRect.width()-QApplication::style()->pixelMetric(QStyle::PM_MenuButtonIndicator));

         PacmanItemModel * model = (PacmanItemModel *)_model;
         isArrowPressed = false;
         if (!model->row(index).isChosen()) {
             if (event->type() == QEvent::MouseButtonRelease) {
                 isArrowPressed = arrowRect.contains(((QMouseEvent *)event)->pos());
             }
             else if (event->type() == QEvent::KeyPress) {
                 isArrowPressed = (((QKeyEvent *)event)->key() == Qt::Key_Right);
             }
         }

         PacmanView * view = (PacmanView *)parent();
         if (isArrowPressed) {
            view->update(index);
            QMenu menu;
            menu.grabKeyboard();
            menu.setMinimumWidth(option.rect.width());
            QList<PacmanItemModel::ChangeMenuParam> menu_params = model->changeStateParamsForMenu(index);
            for (int i=0;i<menu_params.count();i++) {
                menu.addAction(menu_params[i].icon,menu_params[i].text)->setData(QVariant::fromValue<MenuItemData>(MenuItemData(index,menu_params[i].status,model)));
            }
            connect(&menu,SIGNAL(triggered(QAction *)),this,SLOT(menu_triggered(QAction *)));
            menu.exec(view->viewport()->mapToGlobal(QPoint(option.rect.x(),option.rect.y()+option.rect.height())));
            menu.releaseKeyboard();
            isArrowPressed = false;
            view->update(index);
         }
         else {
            PacmanItemModel * model = (PacmanItemModel *)_model;
            model->chooseRow(index,!model->row(index).isChosen());
            view->update(index);
            view->update(model->index(index.row(),0));
            emit rowChoosingStateChanged(index);
         }
     }
     return false;
}

void InstallButtonDelegate::menu_triggered(QAction * action) {
    PacmanView * view = (PacmanView *)parent();
    MenuItemData data = action->data().value<MenuItemData>();
    model->setData(data.index,data.status,Qt::DisplayRole);
    model->chooseRow(data.index,true);
    view->update(data.index);
    emit rowChoosingStateChanged(data.index);
}

void InstallButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect,option.palette.highlight());
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    }
    else {
        int row = ((PacmanView *)parent())->visibleRowIndex(model->index(index.row(),0));
        if (row != -1) painter->fillRect(option.rect,option.palette.color(!fmod(row,2)?QPalette::AlternateBase:QPalette::Base));
        painter->setPen(option.palette.color(QPalette::Text));
    }

    painter->setFont(font);

    if(index.column() != lastColumn) {
         if(index.column() == 0) {
             painter->drawPixmap(0,option.rect.y(),model->data(index,Qt::DecorationRole).value<QIcon>().pixmap(22,22));
             painter->setFont(bfont);
             painter->drawText(option.rect.x()+option.decorationSize.width()+4,option.rect.y(),option.rect.width()-option.decorationSize.width(),option.rect.height(),Qt::AlignLeft|Qt::AlignVCenter,model->data(index,Qt::DisplayRole).toString());
             painter->setFont(font);
         }
         else {
             QRect textrect(option.rect.x()+2,option.rect.y(),option.rect.width()-2,option.rect.height());
             QString text = option.fontMetrics.elidedText(model->data(index,Qt::DisplayRole).toString(),Qt::ElideMiddle,textrect.width());
             painter->drawText(textrect,Qt::AlignLeft|Qt::AlignVCenter,text);
         }
     }
     else {
        PacmanEntry row = model->row(index);
        if ((option.state & QStyle::State_Selected) || row.isChosen()) {
            QStyleOptionToolButton callOptionButton;
            callOptionButton.direction = qApp->layoutDirection();
            callOptionButton.features = QStyleOptionToolButton::MenuButtonPopup;
            callOptionButton.palette = qApp->palette();
            callOptionButton.rect = option.rect;
            callOptionButton.text = model->data(index,Qt::DisplayRole).toString();
            callOptionButton.state = QStyle::State_AutoRaise | QStyle::State_MouseOver | QStyle::State_Enabled | QStyle::State_Raised;
            callOptionButton.toolButtonStyle = Qt::ToolButtonTextBesideIcon;
            if(row.isChosen()) {
                callOptionButton.state &= ~QStyle::State_Raised;
                callOptionButton.state |= QStyle::State_Sunken | QStyle::State_On;
                if (isArrowPressed) callOptionButton.state &= ~QStyle::State_On;
            }
            else {
                if (isArrowPressed) {
                    callOptionButton.state &= ~QStyle::State_Raised;
                    callOptionButton.state |= QStyle::State_Sunken;
                }
            }

            callOptionButton.icon = model->data(index,Qt::DecorationRole).value<QIcon>();
            callOptionButton.iconSize = QSize(22, 22);
            QApplication::style()->drawComplexControl(QStyle::CC_ToolButton, &callOptionButton, painter);
            if (QApplication::style()->objectName().toLower() == "qtcurve") {
                callOptionButton.toolButtonStyle = Qt::ToolButtonTextOnly;
                int delta = callOptionButton.iconSize.width()-QApplication::style()->pixelMetric(QStyle::PM_MenuButtonIndicator,&callOptionButton)+2;
                callOptionButton.rect = QRect(option.rect.x()+delta,option.rect.y(),option.rect.width()-delta,option.rect.height());
                QApplication::style()->drawControl(QStyle::CE_ToolButtonLabel, &callOptionButton, painter);
            }
         }
     }
}
