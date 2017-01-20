/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "filtertoolbutton.h"
#include <QMenu>
#include "treeitemsmenu.h"

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
    setIcon(QIcon(":/pics/view-filter.png"));
    setToolTip(tr("Filters the packages by different groups"));
    is_sel = IS_ALL;
}

FilterToolButton::ItemId FilterToolButton::getSelectedId() {
    return is_sel;
}

void FilterToolButton::fill(const QStringList & _groups) {
    QString text = this->text();
    FilterToolButton::ItemId id = getSelectedId();
    QStringList groups = _groups;
    groups.sort();

    QMenu * menu = new TreeItemsMenu(this);
    QMenu * state_menu = new QMenu(menu);
    QString all_str = tr("All");
    IdAction::addAction(IS_ALL,QIcon(":/pics/folder-tar.png"),all_str,state_menu);
    IdAction::addAction(IS_INSTALLED,QIcon(":/pics/dialog-ok-apply.png"),tr("Installed"),state_menu);
    IdAction::addAction(IS_NONINSTALLED,QIcon(":/pics/notinstalled.png"),tr("Non-Installed"),state_menu);
    IdAction::addAction(IS_NEEDUPDATE,QIcon(":/pics/distro-upgrade.png"),tr("Updates"),state_menu);
    IdAction::addAction(IS_ORPHANED,QIcon(":/pics/dialog-ok-apply.png"),tr("Orphaned"),state_menu);
    IdAction::addAction(IS_MARKED,QIcon(":/pics/edit-select-all.png"),tr("Marked"),state_menu);
    state_menu->setTitle(tr("By state"));
    state_menu->setIcon(QIcon(":/pics/view-filter.png"));
    menu->addMenu(state_menu);
    connect(this,SIGNAL(triggered(QAction *)),this,SLOT(onMenuItemSelected(QAction *)));

    QIcon icon(":/pics/package-group.png");
    QMenu * group_menu = new QMenu(menu);
    group_menu->setTitle(tr("By group"));
    group_menu->setIcon(icon);

    for (int i=0;i<groups.count();i++) {
        IdAction::addAction(IS_GROUP,icon,groups[i],group_menu);
    }
    menu->addMenu(group_menu);

    is_sel = IS_ALL;
    QMenu * old_menu = this->menu();
    setMenu(menu);
    if (old_menu != NULL) delete old_menu;

    setText(text);
    if (!setFilter(id,text)) setText(all_str);
}

bool FilterToolButton::setFilter(FilterToolButton::ItemId id,const QString & filter) {
    QList<QAction *> actions = menu()->actions();
    for (int i=0;i<actions.count();i++) {
        ItemId item_id = ((actions[i]->menu() == NULL)?((IdAction *)actions[i])->id():IS_UNKNOWN);

        if (actions[i]->menu() == NULL) {
            bool is_ok = ((id == IS_GROUP) && (id == item_id) && (actions[i]->text() == filter));
            if (!is_ok) is_ok = ((id != IS_UNKNOWN) && (id != IS_GROUP) && (id == item_id));
            if (is_ok) {
                actions[i]->trigger();
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

bool FilterToolButton::setFilter(QMenu * menu,FilterToolButton::ItemId id,const QString & filter) {
    QList<QAction *> actions = menu->actions();
    for (int i=0;i<actions.count();i++) {
        ItemId item_id = ((actions[i]->menu() == NULL)?((IdAction *)actions[i])->id():IS_UNKNOWN);

        if (actions[i]->menu() == NULL) {
            bool is_ok = ((id == IS_GROUP) && (id == item_id) && (actions[i]->text() == filter));
            if (!is_ok) is_ok = ((id != IS_UNKNOWN) && (id != IS_GROUP) && (id == item_id));
            if (is_ok) {
                actions[i]->trigger();
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
    is_sel = ((action->menu() == NULL)?((IdAction *)action)->id():IS_UNKNOWN);
    emit selected(getSelectedId(),action->iconText());
}

void FilterToolButton::triggerSignal(QAction * action) {
    emit triggered(action);
}
