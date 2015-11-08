/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef CATEGORYTOOLBUTTON_H
#define CATEGORYTOOLBUTTON_H

#include "combotoolbutton.h"
#include <QMenu>

class CategoryToolButton : public ComboToolButton {
    Q_OBJECT
public:
    enum ItemId {
        IS_UNKNOWN = -1,
        IS_DESC = 0,
        IS_NAME = 1,
        IS_FILE_NAME = 2,
        IS_PROVIDER = 3
    };

    explicit CategoryToolButton(QWidget *parent = 0);
    CategoryToolButton::ItemId getSelectedId();

protected slots:
    void onSearchByName();
    void onSearchByProvider();
    void onSearchByDesc();
    void onSearchByFileName();
    void onMenuItemSelected(QAction * action);

signals:
    void selected(CategoryToolButton::ItemId sel);

private:
    QMenu menu;
    CategoryToolButton::ItemId is_sel;
};

#endif // CATEGORYTOOLBUTTON_H
