/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "searchwidget.h"
#include "ui_searchwidget.h"

SearchWidget::SearchWidget(QWidget *parent) : ToolBarWidget(parent), ui(new Ui::SearchWidget) {
    ui->setupUi(this);

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
