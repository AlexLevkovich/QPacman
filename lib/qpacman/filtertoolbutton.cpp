/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "filtertoolbutton.h"
#include <QMenu>
#include "treeitemsmenu.h"
#include "themeicons.h"
#include <unistd.h>

class IdAction : public QAction {
protected:
    IdAction(FilterToolButton::ItemId id,const QString & text,QMenu * parent): QAction(text,parent) {
        m_id = id;
        parent->insertAction(NULL,this);
    }

    IdAction(FilterToolButton::ItemId id,const QIcon & icon,const QString & text,QMenu * parent): QAction(icon,text,parent) {
        m_id = id;
        parent->insertAction(NULL,this);
    }

private:
    FilterToolButton::ItemId m_id;

public:
    FilterToolButton::ItemId id() { return m_id; }

    static IdAction * addAction(FilterToolButton::ItemId id,const QString & text,QMenu * parent) {
        return new IdAction(id,text,parent);
    }

    static IdAction * addAction(FilterToolButton::ItemId id,const QIcon & icon,const QString & text,QMenu * parent) {
        return new IdAction(id,icon,text,parent);
    }
};


FilterToolButton::FilterToolButton(QWidget *parent) : ComboToolButton(parent) {
    setIcon(ThemeIcons::get(ThemeIcons::FILTER));
    setToolTip(tr("Filters the packages by different groups"));
}

FilterToolButton::ItemId FilterToolButton::getSelectedId() const {
    return (currentAction() == NULL)?IS_ALL:((currentAction()->menu() == NULL)?((IdAction *)currentAction())->id():IS_UNKNOWN);
}

QString FilterToolButton::getSelectedFilter() const {
    return (currentAction() == NULL)?"":((currentAction()->menu() == NULL)?((IdAction *)currentAction())->text():currentAction()->menu()->title());
}

void FilterToolButton::fill(const QStringList & _groups) {
    QString text = this->text();
    FilterToolButton::ItemId id = getSelectedId();
    QStringList groups = _groups;
    groups.sort();

    QMenu * menu = new TreeItemsMenu(this);
    QMenu * state_menu = new QMenu(menu);
    QString all_str = tr("All");
    IdAction::addAction(IS_ALL,ThemeIcons::get(ThemeIcons::PKG_GROUP),all_str,state_menu);
    IdAction::addAction(IS_INSTALLED,ThemeIcons::get(ThemeIcons::PKG_INSTALLED_MARK),tr("Installed"),state_menu);
    IdAction::addAction(IS_NONINSTALLED,ThemeIcons::get(ThemeIcons::PKG_NONINSTALLED_MARK),tr("Non-Installed"),state_menu);
    IdAction::addAction(IS_NEEDUPDATE,ThemeIcons::get(ThemeIcons::UPDATE_ITEM),tr("Updates"),state_menu);
    IdAction::addAction(IS_ORPHANED,ThemeIcons::get(ThemeIcons::PKG_INSTALLED_MARK),tr("Orphaned"),state_menu);
    IdAction::addAction(IS_MARKED,ThemeIcons::get(ThemeIcons::PKG_SELECT_ALL),tr("Marked"),state_menu);
    state_menu->setTitle(tr("By state"));
    state_menu->setIcon(ThemeIcons::get(ThemeIcons::FILTER));
    menu->addMenu(state_menu);
    connect(this,&FilterToolButton::triggered,this,&FilterToolButton::onMenuItemSelected);

    QIcon icon = ThemeIcons::get(ThemeIcons::PKG_GROUP);
    QMenu * group_menu = new QMenu(menu);
    group_menu->setTitle(tr("By group"));
    group_menu->setIcon(icon);

    for (int i=0;i<groups.count();i++) {
        IdAction::addAction(IS_GROUP,icon,groups[i],group_menu);
    }
    menu->addMenu(group_menu);

    QMenu * old_menu = this->menu();
    setMenu(menu);
    if (old_menu != NULL) delete old_menu;

    setText(text);
    if (!setFilter(id,text)) setText(all_str);
}

bool FilterToolButton::setFilter(FilterToolButton::ItemId id,const QString & filter) {
    return setFilter(menu()->actions(),id,filter);
}

bool FilterToolButton::setFilter(QMenu * menu,FilterToolButton::ItemId id,const QString & filter) {
    return setFilter(menu->actions(),id,filter);
}

bool FilterToolButton::setFilter(const QList<QAction *> & actions,FilterToolButton::ItemId id,const QString & filter) {
    for (int i=0;i<actions.count();i++) {
        ItemId item_id = ((actions[i]->menu() == NULL)?((IdAction *)actions[i])->id():IS_UNKNOWN);

        if (actions[i]->menu() == NULL) {
            QString filter_str = actions[i]->text();
            if (filter_str.contains("&")) filter_str = filter_str.replace("&","");
            bool is_ok = ((id == IS_GROUP) && (id == item_id) && (filter_str == filter));
            if (!is_ok) is_ok = ((id != IS_UNKNOWN) && (id != IS_GROUP) && (id == item_id));
            if (is_ok) {
                actions[i]->trigger();
                setText(filter_str);
                triggerSignal(actions[i]);
                return true;
            }
        }
        else if (actions[i]->menu() != NULL) {
            if (setFilter(actions[i]->menu(),id,filter)) return true;
        }
    }
    return false;
}

void FilterToolButton::onMenuItemSelected(QAction * action) {
    QString filter = action->iconText();
    if (filter.contains("&")) filter = filter.replace("&","");
    emit selected(getSelectedId(),filter);
}

void FilterToolButton::triggerSignal(QAction * action) {
    emit triggered(action);
}
