/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef FILTERTOOLBUTTON_H
#define FILTERTOOLBUTTON_H

#include "combotoolbutton.h"

class FilterToolButton : public ComboToolButton {
    Q_OBJECT
public:
    enum ItemId {
        IS_UNKNOWN = -1,
        IS_ALL = 0,
        IS_INSTALLED = 1,
        IS_NONINSTALLED = 2,
        IS_NEEDUPDATE = 3,
        IS_ORPHANED = 4,
        IS_GROUP = 5,
        IS_MARKED = 6
    };

    explicit FilterToolButton(QWidget *parent = 0);
    FilterToolButton::ItemId getSelectedId();
    void fill(const QStringList & groups);
    void triggerSignal(QAction * action);
    bool setFilter(FilterToolButton::ItemId id,const QString & filter);

protected slots:
    void onMenuItemSelected(QAction * action);

signals:
    void selected(FilterToolButton::ItemId sel,const QString & group);

private:
    FilterToolButton::ItemId is_sel;

    bool setFilter(QMenu * menu,FilterToolButton::ItemId id,const QString & filter);
};

#endif // FILTERTOOLBUTTON_H
