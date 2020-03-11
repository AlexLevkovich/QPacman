/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "searchwidget.h"
#include "static.h"
#include "themeicons.h"
#include "ui_searchwidget.h"

SearchWidget::SearchWidget(QWidget *parent) : QWidget(parent), ui(new Ui::SearchWidget) {
    ui->setupUi(this);

    ui->categoryButton->setIcon(ThemeIcons::get(ThemeIcons::FIND));
    ui->repoButton->setIcon(ThemeIcons::get(ThemeIcons::REPO));
    ui->filterButton->setIcon(ThemeIcons::get(ThemeIcons::FILTER));

    ui->categoryButton->setIconSize(Static::quadroSize(fontMetrics().height()+4));
    ui->repoButton->setIconSize(ui->categoryButton->iconSize());
    ui->filterButton->setIconSize(ui->categoryButton->iconSize());

    connect(ui->filterButton,SIGNAL(selected(FilterToolButton::ItemId,const QString &)),this,SLOT(onSelectedFilter(FilterToolButton::ItemId,const QString &)));
    connect(ui->repoButton,SIGNAL(selected(const QString &)),this,SLOT(onSelectedRepo(const QString &)));
    connect(ui->categoryButton,SIGNAL(selected(CategoryToolButton::ItemId)),this,SLOT(onSelectedCategory(CategoryToolButton::ItemId)));
    connect(ui->searchBox,SIGNAL(changed(const QString &)),this,SLOT(onTextChanged(const QString &)));
}

SearchWidget::~SearchWidget() {
    delete ui;
}

void SearchWidget::fillFiltersInCombo(const QStringList & groups) {
    ui->filterButton->fill(groups);
}

void SearchWidget::fillReposInCombo(const QStringList & repos) {
    ui->repoButton->fill(repos);
}

void SearchWidget::onSelectedFilter(FilterToolButton::ItemId itemId,const QString & group) {
    emit search_changed(ui->searchBox->text(),ui->categoryButton->getSelectedId(),itemId,group,ui->repoButton->iconText());
}

void SearchWidget::onSelectedRepo(const QString & repo) {
    emit search_changed(ui->searchBox->text(),ui->categoryButton->getSelectedId(),ui->filterButton->getSelectedId(),ui->filterButton->iconText(),repo);
}

void SearchWidget::onSelectedCategory(CategoryToolButton::ItemId itemId) {
    emit search_changed(ui->searchBox->text(),itemId,ui->filterButton->getSelectedId(),ui->filterButton->iconText(),ui->repoButton->iconText());
}

void SearchWidget::onTextChanged(const QString & text) {
    emit search_changed(text,ui->categoryButton->getSelectedId(),ui->filterButton->getSelectedId(),ui->filterButton->iconText(),ui->repoButton->iconText());
}

void SearchWidget::clearSearchText() {
    ui->searchBox->clear();
}

void SearchWidget::clearFilter() {
    ui->filterButton->setFilter(FilterToolButton::IS_ALL,"");
}

bool SearchWidget::setFilter(FilterToolButton::ItemId id,const QString & filter) {
    return ui->filterButton->setFilter(id,filter);
}

QString SearchWidget::searchText() const {
    return ui->searchBox->text();;
}

QString SearchWidget::filterText() const {
    return ui->filterButton->text();
}

QString SearchWidget::repo() const {
    return ui->repoButton->text();
}

FilterToolButton::ItemId SearchWidget::filter() const {
    return ui->filterButton->getSelectedId();
}

CategoryToolButton::ItemId SearchWidget::category() const {
    return ui->categoryButton->getSelectedId();
}
