/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "installbuttondelegate.h"
#include <QMouseEvent>
#include <QPainter>
#include <QMenu>
#include <QApplication>
#include "packageview.h"
#include "static.h"
#include <math.h>
#include "optionsmenu.h"
#include "optionswidget.h"
#include <QDebug>

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

int InstallButtonDelegate::lastColumn = 4;

InstallButtonDelegate::InstallButtonDelegate(QObject *parent) : QStyledItemDelegate(parent) {
    isArrowPressed = false;
    m_text_width = -1;
    firsttime = true;
}

QSize InstallButtonDelegate::sizeHint(const QStyleOptionViewItem & option,const QModelIndex & index) const {
    if (!index.isValid()) return QSize(0,0);

    InstallButtonDelegate * p_this = (InstallButtonDelegate *)this;
    PackageView * view = qobject_cast<PackageView*>((QObject *)option.widget);
    if (view != NULL && firsttime) {
        p_this->firsttime = false;
        qRegisterMetaType<QItemSelection>("QItemSelection");
        connect(view->selectionModel(),&QItemSelectionModel::selectionChanged,this,[=](const QItemSelection & selected,const QItemSelection &) { p_this->item_selected(selected,view); });
    }

    PackageItemModel * model = (PackageItemModel *)index.model();

    if (index.column() < lastColumn) {
        return model->data(index,Qt::SizeHintRole).toSize();
    }
    else {
        int height = max(option.fontMetrics.height(),option.decorationSize.height()) - QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth)*2;
        ((InstallButtonDelegate *)this)->m_text_width = (m_text_width < 0)?OptionsWidget::maxTextWidth(option.font):m_text_width;
        int width = m_text_width + option.fontMetrics.horizontalAdvance(' ')*2;
        width += option.decorationSize.width();
        width += QApplication::style()->pixelMetric(QStyle::PM_MenuButtonIndicator);
        return QApplication::style()->sizeFromContents(QStyle::CT_ToolButton,&option,QSize(width,height));
    }
}

bool InstallButtonDelegate::editorEvent(QEvent *event,QAbstractItemModel *_model,const QStyleOptionViewItem &option,const QModelIndex &index) {
    PackageItemModel * model = (PackageItemModel *)_model;
    PackageView * view = qobject_cast<PackageView*>((QObject *)option.widget);
    if (view == NULL) return false;

    if(((event->type() == QEvent::MouseButtonRelease) && (index.column() == lastColumn)) || ((event->type() == QEvent::KeyPress) && (((QKeyEvent *)event)->modifiers() & Qt::ShiftModifier) && (((QKeyEvent *)event)->key() == Qt::Key_Right))) {
         QRect arrowRect = option.rect;
         arrowRect.setX(arrowRect.x()+arrowRect.width()-QApplication::style()->pixelMetric(QStyle::PM_MenuButtonIndicator));

         isArrowPressed = false;
         if (event->type() == QEvent::MouseButtonRelease) {
             isArrowPressed = arrowRect.contains(((QMouseEvent *)event)->pos());
         }
         else if (event->type() == QEvent::KeyPress) isArrowPressed = true;

         if (isArrowPressed) {
            view->update(index);
            qRegisterMetaType<PackageView *>("PackageView *");
            QMetaObject::invokeMethod(this,"show_menu",Qt::QueuedConnection,Q_ARG(AlpmPackage,model->row(index)),Q_ARG(QModelIndex,index),Q_ARG(QRect,option.rect),Q_ARG(PackageView *,view));
         }
         else {
            if (prev_index == index && prev_index == model->index(curr_index.row(),lastColumn)) {
                model->chooseRow(index,model->row(index).changeStatus() == AlpmPackage::DO_NOTHING);
                view->update(index);
                view->update(model->index(index.row(),0));
                emit rowChoosingStateChanged(index);
            }
            else prev_index = model->index(index.row(),lastColumn);
         }
     }
     else if (event->type() == QEvent::MouseButtonRelease) {
        prev_index = model->index(index.row(),lastColumn);
        curr_index = prev_index;
    }
     return false;
}

void InstallButtonDelegate::show_menu(const AlpmPackage & pkg,const QModelIndex & index,const QRect & rect,PackageView * view) {
    QMenu * menu = NULL;
    if (!pkg.isInstalled()) menu = new InstallOptionsMenu(pkg.changeStatus(),view);
    else if (pkg.isInstalled() && (pkg.repo() == "local" || pkg.repo() == "aur")) menu = new UninstallOptionsMenu(pkg.changeStatus(),view);
    else menu = new ReinstallOptionsMenu(pkg.changeStatus(),view);
    menu->grabKeyboard();
    menu->setMinimumWidth(rect.width());
    menu->setProperty("menu_index",index);
    connect(menu,&QMenu::triggered,this,&InstallButtonDelegate::menu_triggered);
    menu->exec(view->viewport()->mapToGlobal(QPoint(rect.x(),rect.y()+rect.height())));
    menu->releaseKeyboard();
    menu->deleteLater();
    isArrowPressed = false;
}

void InstallButtonDelegate::menu_triggered(QAction * action) {
    QMenu * menu = (QMenu *)QObject::sender();
    QAbstractItemView * view = (QAbstractItemView *)menu->parent();
    QModelIndex menu_index = menu->property("menu_index").toModelIndex();
    PackageItemModel * model = (PackageItemModel *)menu_index.model();
    model->setData(menu_index,(AlpmPackage::UserChangeStatus)action->data().toInt(),Qt::DisplayRole);
    model->chooseRow(menu_index,true,(AlpmPackage::UserChangeStatus)action->data().toInt());
    view->update(menu_index);
    emit rowChoosingStateChanged(menu_index);
}

void InstallButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &_option, const QModelIndex &index) const {
    QStyleOptionViewItem option = _option;
    PackageItemModel * model = (PackageItemModel *)index.model();
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect,option.palette.highlight());
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    }
    else {
        painter->fillRect(option.rect,option.palette.color(!fmod(index.row(),2)?QPalette::AlternateBase:QPalette::Base));
        painter->setPen(option.palette.color(QPalette::Text));
    }

    option.font.setBold(false);
    painter->setFont(option.font);

    if(index.column() != lastColumn) {
         if(index.column() == 0) {
             painter->drawPixmap(0,option.rect.y(),model->data(index,Qt::DecorationRole).value<QIcon>().pixmap(quadroSize(option.fontMetrics.height()+4)));
             option.font.setBold(true);
             painter->setFont(option.font);
             painter->drawText(option.rect.x()+option.decorationSize.width()+4,option.rect.y(),option.rect.width()-option.decorationSize.width(),option.rect.height(),Qt::AlignLeft|Qt::AlignVCenter,model->data(index,Qt::DisplayRole).toString());
         }
         else {
             QRect textrect(option.rect.x()+2,option.rect.y(),option.rect.width()-2,option.rect.height());
             QString text = option.fontMetrics.elidedText(model->data(index,Qt::DisplayRole).toString(),Qt::ElideMiddle,textrect.width());
             painter->drawText(textrect,Qt::AlignLeft|Qt::AlignVCenter,text);
         }
     }
     else {
        AlpmPackage row = model->row(index);
        if (!row.isValid()) return;
        if ((option.state & QStyle::State_Selected) || row.changeStatus() != AlpmPackage::DO_NOTHING) {
            QStyleOptionToolButton callOptionButton;
            callOptionButton.direction = Qt::LeftToRight;
            callOptionButton.features = QStyleOptionToolButton::MenuButtonPopup;
            callOptionButton.palette = qApp->palette();
            callOptionButton.rect = option.rect;
            callOptionButton.icon = QIcon();
            callOptionButton.iconSize = QSize(option.rect.height(),option.rect.height());
            callOptionButton.text = "";
            callOptionButton.state = QStyle::State_AutoRaise | QStyle::State_MouseOver | QStyle::State_Enabled | QStyle::State_Raised;
            callOptionButton.toolButtonStyle = Qt::ToolButtonTextBesideIcon;
            if(row.changeStatus() != AlpmPackage::DO_NOTHING) {
                callOptionButton.state &= ~QStyle::State_Raised;
                callOptionButton.state |= QStyle::State_Sunken | QStyle::State_On;
                if (isArrowPressed) callOptionButton.state &= ~QStyle::State_On;
            }
            else {
                if (isArrowPressed) {
                    callOptionButton.state &= ~QStyle::State_Raised;
                    callOptionButton.state |= QStyle::State_Sunken;
                }
                else callOptionButton.state &= ~QStyle::State_MouseOver;
            }

            QApplication::style()->drawComplexControl(QStyle::CC_ToolButton, &callOptionButton, painter);
            painter->drawPixmap(option.rect.x(),option.rect.y(),model->data(index,Qt::DecorationRole).value<QIcon>().pixmap(callOptionButton.iconSize));
            painter->drawText(option.rect.x()+option.decorationSize.width()+4,option.rect.y(),option.rect.width()-option.decorationSize.width(),option.rect.height(),Qt::AlignLeft|Qt::AlignVCenter,model->data(index,Qt::DisplayRole).toString());
         }
     }
}

void InstallButtonDelegate::item_selected(const QItemSelection & selection,PackageView * view) {
    if (selection.isEmpty()) return;
    QModelIndex index = view->model()->index(selection.indexes().at(0).row(),0);
    QRect rect = view->visualRect(view->model()->index(selection.indexes().at(0).row(),lastColumn));
    if (QRect(view->viewport()->mapToGlobal(rect.topLeft()),rect.size()).contains(QCursor::pos())) {
        curr_index = index;
        return;
    }
    QStyleOptionViewItem option;
    option.widget = view;
    option.rect = view->visualRect(index);
    QMouseEvent m_event(QEvent::MouseButtonRelease,option.rect.topLeft(),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    editorEvent(&m_event,view->model(),option,index);
}

bool InstallButtonDelegate::eventFilter(QObject *editor,QEvent *event) {
     PackageView * view = qobject_cast<PackageView*>(editor);
     if (view == NULL || (event->type() != QEvent::KeyPress)) return false;

     QKeyEvent * e = (QKeyEvent *)event;
     QModelIndex index = view->selectedRow();
     if (!index.isValid()) return false;
     QStyleOptionViewItem option;
     option.widget = view;

     switch (e->key()) {
         case Qt::Key_Return:
         case Qt::Key_Enter:
         {
             if (!(e->modifiers() & Qt::ShiftModifier)) break;
             index = view->model()->index(index.row(),view->model()->columnCount()-1);
             option.rect = view->visualRect(index);
             QMouseEvent m_event(QEvent::MouseButtonRelease,option.rect.topLeft(),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
             editorEvent(&m_event,view->model(),option,index);
             break;
         }
         case Qt::Key_Right:
         {
             if (!(e->modifiers() & Qt::ShiftModifier)) break;
             index = view->model()->index(index.row(),view->model()->columnCount()-1);
             option.rect = view->visualRect(index);
             editorEvent(event,view->model(),option,index);
             break;
         }
         default:
             break;
     }

     return false;
}
