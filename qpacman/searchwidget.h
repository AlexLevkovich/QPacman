/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

namespace Ui {
class SearchWidget;
}

#include "filtertoolbutton.h"
#include "categorytoolbutton.h"

class SearchWidget : public QWidget {
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = 0);
    ~SearchWidget();
    void fillFiltersInCombo(const QStringList & groups);
    void fillReposInCombo(const QStringList & repos);
    void setSearchText(const QString & text);
    void clearSearchText();
    void setCategory(CategoryToolButton::ItemId id);
    void clearFilter();
    void setAllRepos();
    void setRepo(const QString & repo);
    bool setFilter(FilterToolButton::ItemId id,const QString & filter);

    QString searchText() const;
    QString filterText() const;
    QString repo() const;
    FilterToolButton::ItemId filter() const;
    CategoryToolButton::ItemId category() const;

protected slots:
    void onSelectedFilter(FilterToolButton::ItemId itemId,const QString & group);
    void onSelectedRepo(const QString & repo);
    void onSelectedCategory(CategoryToolButton::ItemId itemId);
    void onTextChanged(const QString & text);

signals:
    void search_changed(const QString &,CategoryToolButton::ItemId,FilterToolButton::ItemId,const QString &,const QString &);

private:
    Ui::SearchWidget *ui;
};

#endif // SEARCHWIDGET_H
