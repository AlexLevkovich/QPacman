/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef CATEGORYLISTVIEW_H
#define CATEGORYLISTVIEW_H

#include <QListView>

class CategoryWidget : public QWidget {
    Q_OBJECT
public:
    CategoryWidget(QWidget *parent = nullptr) : QWidget(parent) {}
public slots:
    virtual void okPressed() = 0;
};

Q_DECLARE_METATYPE(CategoryWidget*)

class CategoryListView : public QListView {
    Q_OBJECT
public:
    CategoryListView(QWidget *parent = 0);
    void addCategory(const QString & displayName,CategoryWidget *widget,const QIcon & icon = QIcon());
    bool select(CategoryWidget *widget);
    bool select(const QString & displayName);

protected:
    virtual QSize sizeHint() const;
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual bool event(QEvent *event);
    virtual void selectionChanged(const QItemSelection & selected,const QItemSelection & deselected);
};

#endif // CATEGORYLISTVIEW_H
