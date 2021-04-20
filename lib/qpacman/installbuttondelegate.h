/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef INSTALLBUTTONDELEGATE_H
#define INSTALLBUTTONDELEGATE_H

#include <QStyledItemDelegate>
#include <QToolButton>
#include <QItemSelection>

class PackageView;
class AlpmPackage;

class InstallButtonDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit InstallButtonDelegate(QObject *parent = 0);
    QSize sizeHint(const QStyleOptionViewItem & option,const QModelIndex & index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:
    void rowChoosingStateChanged(const QModelIndex &index);

protected:
    bool editorEvent(QEvent *event,QAbstractItemModel *_model,const QStyleOptionViewItem & option,const QModelIndex &index);
    bool eventFilter(QObject *editor,QEvent *event);

private slots:
    void menu_triggered(QAction * action);
    void show_menu(const AlpmPackage & pkg,const QModelIndex & index,const QRect & rect,PackageView * view);
    void item_selected(const QItemSelection & selected,PackageView * view);

private:
    static int lastColumn;
    bool isArrowPressed;
    QModelIndex curr_index;
    QModelIndex prev_index;
    int m_text_width;
    bool firsttime;

    friend class PackageView;
};

#endif // INSTALLBUTTONDELEGATE_H
