/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

namespace Ui {
class SearchWidget;
}

#include "toolbarwidget.h"
#include "filtertoolbutton.h"
#include "categorytoolbutton.h"

class SearchWidget : public ToolBarWidget {
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = 0);
    ~SearchWidget();
    void fillFiltersInCombo(const QStringList & groups);
    void fillReposInCombo(const QStringList & repos);
    void clearSearchText();
    void clearFilter();
    bool setFilter(FilterToolButton::ItemId id,const QString & filter);
    void saveFilter();
    void restoreFilter();
    void saveRepo();
    void restoreRepo();
    void saveSearchText();
    void restoreSearchText();
    void saveAllFilters();
    void restoreAllFilters();
    FilterToolButton::ItemId filter();

protected slots:
    void onSelectedFilter(FilterToolButton::ItemId itemId,const QString & group);
    void onSelectedRepo(const QString & repo);
    void onSelectedCategory(CategoryToolButton::ItemId itemId);
    void onTextChanged(const QString & text);

signals:
    void search_changed(const QString &,CategoryToolButton::ItemId,FilterToolButton::ItemId,const QString &,const QString &);

private:
    Ui::SearchWidget *ui;
    QString m_saveSearchText;
    QString m_saveFilter;
    QString m_saveRepo;
    FilterToolButton::ItemId m_saveFilterId;
};

#endif // SEARCHWIDGET_H
