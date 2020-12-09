/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "traypreferences.h"
#include "static.h"
#include "libalpm.h"
#include "usualuserupdateschecker.h"
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

    m_use_sound = ui->trayOptions->doPlaySound();
    m_tray = new QPacmanTrayIcon(&m_use_sound,this);

    actionCheck_for_updates = m_tray->checkUpdatesAction();
    actionUpdate_now = m_tray->updateAction();
    actionPreferences = m_tray->preferencesAction();
    actionLoad_QPacman = m_tray->mainWindowAction();
    actionQuit = m_tray->quitAction();

    addToolBar(toolBar = new QToolBar(this));
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->addAction(actionCheck_for_updates);
    toolBar->addAction(actionUpdate_now);
    toolBar->addAction(actionLoad_QPacman);
    toolBar->addAction(actionQuit);

    m_blocking_operation = false;
    m_timer.setSingleShot(true);

    ui->categoryList->addCategory(tr("Program"),ui->trayOptions,ThemeIcons::get(ThemeIcons::QPACMANTRAY));
    ui->categoryList->addCategory("Alpm",ui->alpmOptions,ThemeIcons::get(ThemeIcons::ARCHLINUX));
    ui->categoryList->select(ui->trayOptions);

    connect(actionPreferences,SIGNAL(triggered()),this,SIGNAL(showRequest()));
    connect(actionCheck_for_updates,SIGNAL(triggered()),this,SLOT(onCheckForUpdatesTriggered()));
    connect(actionUpdate_now,SIGNAL(triggered()),this,SLOT(onUpdateNowTriggered()));
    connect(actionLoad_QPacman,SIGNAL(triggered()),this,SLOT(onLoadQPacmanTriggered()));
    connect(actionQuit,SIGNAL(triggered()),this,SLOT(onQuitTriggered()));
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(onCheckForUpdatesTriggered()));
    connect(qApp,SIGNAL(qpacmanStarted(const QStringList &)),this,SLOT(qpacmanStarted(const QStringList &)));
    connect(qApp,SIGNAL(qpacmanEnded(const QStringList &,qint64)),this,SLOT(qpacmanEnded(const QStringList &,qint64)));
    connect(Alpm::instance(),SIGNAL(locking_changed(const QString &,bool)),this,SLOT(updateActions(const QString &,bool)));

    m_timer.start(timeout);
}

TrayPreferences::~TrayPreferences() {
    delete ui;
}

void TrayPreferences::showEvent(QShowEvent * event) {
    QMainWindow::showEvent(event);
    restoreGeometry(Static::iniValue<QByteArray>("geometry/mainwindow",QByteArray()));
}

void TrayPreferences::post_resize_save() {
    Static::setIniValue("geometry/mainwindow",saveGeometry());
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

void TrayPreferences::updateActions(const QString &,bool locked) {
    actionUpdate_now->setEnabled(!m_blocking_operation);
    actionCheck_for_updates->setEnabled(!m_blocking_operation);
    actionQuit->setEnabled(!m_blocking_operation);
    actionLoad_QPacman->setEnabled(!m_blocking_operation);
    if (!m_blocking_operation && Alpm::isOpen()) {
        actionUpdate_now->setEnabled(!locked);
        actionCheck_for_updates->setEnabled(!locked);
        actionQuit->setEnabled(!locked);
    }
}

void TrayPreferences::onCheckForUpdatesTriggered() {
    if (m_blocking_operation) return;

    m_blocking_operation = true;
    updateActions();
    m_tray->checkingInProgress();
    UsualUserUpdatesChecker * checker = new UsualUserUpdatesChecker();
    connect(checker,SIGNAL(ok(const QStringList &)),this,SLOT(checker_ok(const QStringList &)));
    connect(checker,SIGNAL(error(const QString &,int)),this,SLOT(checker_error(const QString &,int)));
}

void TrayPreferences::checker_ok(const QStringList & packages) {
    m_blocking_operation = false;
    updateActions();
    bool tray_visible = m_tray->isVisible();
    m_tray->updatesFound(packages);
    if (tray_visible) m_timer.start(ui->trayOptions->interval()*60000);
}

void TrayPreferences::checker_error(const QString & error,int err_id) {
    m_blocking_operation = false;
    updateActions();
    bool tray_visible = m_tray->isVisible();
    m_tray->checkingCompleted(error,err_id);
    if (tray_visible) m_timer.start(ui->trayOptions->errInterval()*60000);
}

void TrayPreferences::onUpdateNowTriggered() {
    m_blocking_operation = true;
    updateActions();
    if (!Static::startQPacman(QStringList() << "--update",this,SLOT(pacman_finished(int)))) {
        m_blocking_operation = false;
        updateActions();
        m_tray->setVisible(false);
        m_timer.start(ui->trayOptions->errInterval()*60000);
    }
}

void TrayPreferences::pacman_finished(int rc) {
    m_blocking_operation = false;
    qpacmanEnded(QStringList(),rc);
}

void TrayPreferences::onQuitTriggered() {
    qApp->quit();
}

void TrayPreferences::onLoadQPacmanTriggered() {
    Static::startQPacman();
}

void TrayPreferences::qpacmanStarted(const QStringList &) {
    m_tray->setVisible(false);
}

void TrayPreferences::qpacmanEnded(const QStringList & parms,qint64 rc) {
    if (rc != 0 || (parms.count() > 0 && parms[0] == "--user") || !ui->trayOptions->checkUpdatesIfQPacmanUnloaded()) {
        updateActions();
        return;
    }
    m_tray->setVisible(true);
    onCheckForUpdatesTriggered();
}
