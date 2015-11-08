/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef INSTALLBUTTONDELEGATE_H
#define INSTALLBUTTONDELEGATE_H

#include <QStyledItemDelegate>
#include <QToolButton>

class PacmanItemModel;

class InstallButtonDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit InstallButtonDelegate(QObject *parent = 0);
    void setModel(PacmanItemModel * _model);
    QSize sizeHint(const QStyleOptionViewItem & option,const QModelIndex & index) const;
    bool editorEvent(QEvent *event,QAbstractItemModel *_model,const QStyleOptionViewItem & option,const QModelIndex &index);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:
    void rowChoosingStateChanged(const QModelIndex &index);

private slots:
    void menu_triggered(QAction * action);

private:
    PacmanItemModel * model;
    static int lastColumn;
    QFont font;
    QFont bfont;
    bool isArrowPressed;
};

#endif // INSTALLBUTTONDELEGATE_H
