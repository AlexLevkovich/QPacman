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
#include "themeicons.h"
#include "ui_traypreferences.h"
#include "messagedialog.h"
#include <QDebug>

TrayPreferences::TrayPreferences(QWidget *parent) : QMainWindow(parent), ui(new Ui::TrayPreferences) {
    ui->setupUi(this);

    ui->actionCheck_for_updates->setIcon(ThemeIcons::get(ThemeIcons::UPDATE_REPOS));
    ui->actionUpdate_now->setIcon(ThemeIcons::get(ThemeIcons::SYNC));
    ui->actionPreferences->setIcon(ThemeIcons::get(ThemeIcons::CONFIGURE));
    ui->actionLoad_QPacman->setIcon(ThemeIcons::get(ThemeIcons::QPACMAN));
    ui->actionQuit->setIcon(ThemeIcons::get(ThemeIcons::QUIT));

    m_tray = new QPacmanTrayIcon(ui->actionCheck_for_updates,ui->actionUpdate_now,ui->actionPreferences,ui->actionLoad_QPacman,ui->actionQuit,this);

    m_blocking_operation = false;
    m_timer.setSingleShot(true);

    ui->categoryList->addCategory(tr("Program"),ui->trayOptions,ThemeIcons::get(ThemeIcons::QPACMANTRAY));
    ui->categoryList->addCategory("Alpm",ui->alpmOptions,ThemeIcons::get(ThemeIcons::ARCHLINUX));
    ui->categoryList->select(ui->trayOptions);

    connect(&m_timer,SIGNAL(timeout()),this,SLOT(on_actionCheck_for_updates_triggered()));
    connect(ui->actionPreferences,SIGNAL(triggered()),this,SIGNAL(showRequest()));
    connect(qApp,SIGNAL(qpacmanStarted(const QStringList &)),this,SLOT(qpacmanStarted(const QStringList &)));
    connect(qApp,SIGNAL(qpacmanEnded(const QStringList &,qint64)),this,SLOT(qpacmanEnded(const QStringList &,qint64)));
    connect(Alpm::instance(),SIGNAL(locking_changed(const QString &,bool)),this,SLOT(updateActions(const QString &,bool)));

    m_timer.start(0);
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
    setVisible(false);
}

void TrayPreferences::on_buttonBox_rejected() {
    setVisible(false);
}

void TrayPreferences::updateActions(const QString &,bool locked) {
    ui->actionUpdate_now->setEnabled(!m_blocking_operation);
    ui->actionCheck_for_updates->setEnabled(!m_blocking_operation);
    ui->actionQuit->setEnabled(!m_blocking_operation);
    ui->actionLoad_QPacman->setEnabled(!m_blocking_operation);
    if (!m_blocking_operation && Alpm::isOpen()) {
        ui->actionUpdate_now->setEnabled(!locked);
        ui->actionCheck_for_updates->setEnabled(!locked);
        ui->actionQuit->setEnabled(!locked);
    }
}

void TrayPreferences::on_actionCheck_for_updates_triggered() {
    if (m_blocking_operation) return;

    m_blocking_operation = true;
    updateActions();
    m_tray->checkingInProgress();
    UsualUserUpdatesChecker * checker = new UsualUserUpdatesChecker();
    connect(checker,SIGNAL(ok(const QStringList &)),this,SLOT(checker_ok(const QStringList &)));
    connect(checker,SIGNAL(error(const QString &)),this,SLOT(checker_error(const QString &)));
}

void TrayPreferences::checker_ok(const QStringList & packages) {
    m_blocking_operation = false;
    updateActions();
    bool tray_visible = m_tray->isVisible();
    m_tray->updatesFound(packages);
    if (tray_visible) m_timer.start(ui->trayOptions->interval()*60000);
}

void TrayPreferences::checker_error(const QString & error) {
    m_blocking_operation = false;
    updateActions();
    bool tray_visible = m_tray->isVisible();
    m_tray->checkingCompleted(error);
    if (tray_visible) m_timer.start(ui->trayOptions->errInterval()*60000);
}

void TrayPreferences::on_actionUpdate_now_triggered() {
    m_blocking_operation = true;
    updateActions();
    m_tray->updateInProgress();
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

void TrayPreferences::on_actionQuit_triggered() {
    qApp->quit();
}

void TrayPreferences::on_actionLoad_QPacman_triggered() {
    Static::startQPacman();
}

void TrayPreferences::qpacmanStarted(const QStringList &) {
    m_tray->setVisible(false);
}

void TrayPreferences::qpacmanEnded(const QStringList & parms,qint64 rc) {
    if (rc != 0 || (parms.count() > 0 && parms[0] == "--user")) {
        updateActions();
        return;
    }
    m_tray->setVisible(true);
    on_actionCheck_for_updates_triggered();
}
