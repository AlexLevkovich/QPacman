/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "traypreferences.h"
#include "static.h"
#include "libalpm.h"
#include "updatechecker.h"
#include "updater.h"
#include <QShowEvent>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QToolBar>
#include "themeicons.h"
#include "ui_traypreferences.h"
#include "messagedialog.h"
#include <QDebug>

TrayPreferences::TrayPreferences(int timeout,QWidget *parent) : QMainWindow(parent), ui(new Ui::TrayPreferences) {
    ui->setupUi(this);

    updater = nullptr;
    m_use_sound = ui->trayOptions->doPlaySound();
    m_tray = new QPacmanTrayIcon(&m_use_sound,this);

    actionCheck_for_updates = m_tray->checkUpdatesAction();
    actionUpdate_now = m_tray->updateAction();
    actionPreferences = m_tray->preferencesAction();
    actionQuit = m_tray->quitAction();

    addToolBar(toolBar = new QToolBar(this));
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->addAction(actionCheck_for_updates);
    toolBar->addAction(actionUpdate_now);
    toolBar->addAction(actionQuit);

    m_blocking_operation = false;
    m_timer.setSingleShot(true);

    ui->categoryList->addCategory(tr("Program"),ui->trayOptions,ThemeIcons::get(ThemeIcons::QPACMANTRAY));
    ui->categoryList->addCategory("Alpm",ui->alpmOptions,ThemeIcons::get(ThemeIcons::ARCHLINUX));
    ui->categoryList->select(ui->trayOptions);

    connect(actionPreferences,&QAction::triggered,this,&TrayPreferences::showRequest);
    connect(actionCheck_for_updates,&QAction::triggered,this,&TrayPreferences::onCheckForUpdatesTriggered);
    connect(actionUpdate_now,&QAction::triggered,this,&TrayPreferences::onUpdateNowTriggered);
    connect(actionQuit,&QAction::triggered,this,&TrayPreferences::onQuitTriggered);
    connect(&m_timer,&QTimer::timeout,this,&TrayPreferences::onCheckForUpdatesTriggered);

    m_timer.start(timeout);
}

TrayPreferences::~TrayPreferences() {
    delete ui;
}

void TrayPreferences::showEvent(QShowEvent * event) {
    QMainWindow::showEvent(event);
    restoreGeometry(iniValue<QByteArray>("geometry/mainwindow",QByteArray()));
}

void TrayPreferences::post_resize_save() {
    setIniValue("geometry/mainwindow",saveGeometry());
}

void TrayPreferences::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    QMetaObject::invokeMethod(this,"post_resize_save",Qt::QueuedConnection);
}

void TrayPreferences::on_buttonBox_accepted() {
    ui->trayOptions->okPressed();
    ui->alpmOptions->okPressed();
    m_use_sound = ui->trayOptions->doPlaySound();
    setVisible(false);
}

void TrayPreferences::on_buttonBox_rejected() {
    setVisible(false);
}

void TrayPreferences::updateActions() {
    actionUpdate_now->setEnabled(!m_blocking_operation && (updater == nullptr));
    actionCheck_for_updates->setEnabled(!m_blocking_operation);
    actionQuit->setEnabled(!m_blocking_operation);
}

void TrayPreferences::onCheckForUpdatesTriggered() {
    if (m_blocking_operation) return;

    m_blocking_operation = true;
    updateActions();
    m_tray->checkingInProgress();
    UpdateChecker * checker = new UpdateChecker();
    connect(checker,&UpdateChecker::completed,this,&TrayPreferences::checker_completed);
}

void TrayPreferences::checker_completed(ThreadRun::RC ok,const QString & error,const QStringList & updates) {
    m_blocking_operation = false;
    updateActions();
    bool tray_visible = m_tray->isVisible();
    if (ok == ThreadRun::OK) {
        m_tray->updatesFound(updates);
        if (tray_visible) m_timer.start(ui->trayOptions->interval()*60000);
    }
    else {
        m_tray->checkingCompleted(error,0);
        if (tray_visible) m_timer.start(ui->trayOptions->errInterval()*60000);
    }
}

void TrayPreferences::onUpdateNowTriggered() {
    m_blocking_operation = true;
    updateActions();

    updater = new Updater(this);
    connect(updater,&QObject::destroyed,this,[&]() { updater = nullptr; updateActions(); });
    connect(updater,&Updater::completed,this,&TrayPreferences::onUpdaterCompleted);
}

void TrayPreferences::onUpdaterCompleted(ThreadRun::RC rc) {
    m_blocking_operation = false;
    if (rc != ThreadRun::OK) {
        updateActions();
        m_tray->setVisible(false);
        m_timer.start(ui->trayOptions->errInterval()*60000);
    }
    else {
        m_tray->setVisible(true);
        onCheckForUpdatesTriggered();
    }
}

void TrayPreferences::onQuitTriggered() {
    qApp->quit();
}

