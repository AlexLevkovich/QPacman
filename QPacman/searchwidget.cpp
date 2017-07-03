/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "searchwidget.h"
#include "static.h"
#include "ui_searchwidget.h"

SearchWidget::SearchWidget(QWidget *parent) : ToolBarWidget(parent), ui(new Ui::SearchWidget) {
    ui->setupUi(this);

    m_saveFilterId = FilterToolButton::IS_UNKNOWN;

    connect(ui->filterButton,SIGNAL(selected(FilterToolButton::ItemId,const QString &)),this,SLOT(onSelectedFilter(FilterToolButton::ItemId,const QString &)));
    connect(ui->repoButton,SIGNAL(selected(const QString &)),this,SLOT(onSelectedRepo(const QString &)));
    connect(ui->categoryButton,SIGNAL(selected(CategoryToolButton::ItemId)),this,SLOT(onSelectedCategory(CategoryToolButton::ItemId)));
    connect(ui->searchBox,SIGNAL(changed(const QString &)),this,SLOT(onTextChanged(const QString &)));
}

SearchWidget::~SearchWidget() {
    delete ui;
}

FilterToolButton::ItemId SearchWidget::filter() {
    return ui->filterButton->getSelectedId();
}

void SearchWidget::fillFiltersInCombo(const QStringList & groups) {
    ui->filterButton->fill(groups);
}

void SearchWidget::fillReposInCombo(const QStringList & repos) {
    ui->repoButton->fill(repos);
}

void SearchWidget::onSelectedFilter(FilterToolButton::ItemId itemId,const QString & group) {
    QString repo = ui->repoButton->iconText();
    if (repo.contains("&")) repo = repo.replace("&","");
    if (repo == Static::RepoAll_Str) repo.clear();
    emit search_changed(ui->searchBox->text(),ui->categoryButton->getSelectedId(),itemId,group,repo);
}

void SearchWidget::onSelectedRepo(const QString & repo) {
    QString filter = ui->filterButton->iconText();
    if (filter.contains("&")) filter = filter.replace("&","");
    emit search_changed(ui->searchBox->text(),ui->categoryButton->getSelectedId(),ui->filterButton->getSelectedId(),filter,repo);
}

void SearchWidget::onSelectedCategory(CategoryToolButton::ItemId itemId) {
    QString repo = ui->repoButton->iconText();
    if (repo.contains("&")) repo = repo.replace("&","");
    if (repo == Static::RepoAll_Str) repo.clear();
    QString filter = ui->filterButton->iconText();
    if (filter.contains("&")) filter = filter.replace("&","");
    emit search_changed(ui->searchBox->text(),itemId,ui->filterButton->getSelectedId(),filter,repo);
}

void SearchWidget::onTextChanged(const QString & text) {
    QString repo = ui->repoButton->iconText();
    if (repo.contains("&")) repo = repo.replace("&","");
    if (repo == Static::RepoAll_Str) repo.clear();
    QString filter = ui->filterButton->iconText();
    if (filter.contains("&")) filter = filter.replace("&","");
    emit search_changed(text,ui->categoryButton->getSelectedId(),ui->filterButton->getSelectedId(),filter,repo);
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

void SearchWidget::saveFilter() {
    m_saveFilter = ui->filterButton->text();
    m_saveFilterId = ui->filterButton->getSelectedId();
}

void SearchWidget::restoreFilter() {
    if (!setFilter(m_saveFilterId,m_saveFilter)) setFilter(FilterToolButton::IS_ALL,"");
}

void SearchWidget::saveSearchText() {
    m_saveSearchText = ui->searchBox->text();
}

void SearchWidget::restoreSearchText() {
    ui->searchBox->setText(m_saveSearchText);
}

void SearchWidget::saveRepo() {
    m_saveRepo = ui->repoButton->text();
}

void SearchWidget::restoreRepo() {
    ui->repoButton->selectMenuItem(m_saveRepo);
}

void SearchWidget::saveAllFilters() {
    saveFilter();
    saveSearchText();
    saveRepo();
}

void SearchWidget::restoreAllFilters() {
    if (m_saveFilterId == FilterToolButton::IS_UNKNOWN) return;

    restoreFilter();
    restoreSearchText();
    restoreRepo();

    m_saveSearchText.clear();
    m_saveFilter.clear();
    m_saveRepo.clear();
    m_saveFilterId = FilterToolButton::IS_UNKNOWN;
}
