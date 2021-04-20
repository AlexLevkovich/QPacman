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

    ui->categoryButton->setIconSize(quadroSize(fontMetrics().height()+4));
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

void SearchWidget::setSearchText(const QString & text) {
    ui->searchBox->blockSignals(true);
    ui->searchBox->setText(text);
    ui->searchBox->blockSignals(false);
}

void SearchWidget::clearSearchText() {
    ui->searchBox->blockSignals(true);
    ui->searchBox->clear();
    ui->searchBox->blockSignals(false);
}

void SearchWidget::setCategory(CategoryToolButton::ItemId id) {
    ui->categoryButton->blockSignals(true);
    ui->categoryButton->setItemId(id);
    ui->categoryButton->blockSignals(false);
}

void SearchWidget::setAllRepos() {
    ui->repoButton->blockSignals(true);
    ui->repoButton->selectAllMenuItem();
    ui->repoButton->blockSignals(false);
}

void SearchWidget::setRepo(const QString & repo) {
    ui->repoButton->blockSignals(true);
    ui->repoButton->selectMenuItem(repo);
    ui->repoButton->blockSignals(false);
}

void SearchWidget::clearFilter() {
    ui->filterButton->blockSignals(true);
    ui->filterButton->setFilter(FilterToolButton::IS_ALL,"");
    ui->filterButton->blockSignals(false);
}

bool SearchWidget::setFilter(FilterToolButton::ItemId id,const QString & filter) {
    ui->filterButton->blockSignals(true);
    bool ret = ui->filterButton->setFilter(id,filter);
    ui->filterButton->blockSignals(false);
    return ret;
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
