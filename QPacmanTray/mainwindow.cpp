/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QPixmap>
#include <QProcess>
#include <QMessageBox>
#include "pacmanballoontip.h"
#include "pacmandbrefresher.h"
#include "pacmansimpleupdatesreader.h"
#include <QMessageBox>
#include <QPushButton>
#include "errordialog.h"
#include "pacmanerrorballoontip.h"

extern const char * pacmantray_version;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow),
                                          errorIcon(":/pics/kpk-important.png"),
                                          packageIcon(":/pics/distro-upgrade.png"),
                                          good_player(":/sound/KDE-Sys-App-Positive.ogg"),
                                          bad_player(":/sound/KDE-Sys-App-Error.ogg"),
                                          checkingLock("/tmp/QPacmanTray_checker"),
                                          tray(this) {
    ui->setupUi(this);

    refresher = NULL;
    isConnected = true;

    isCheckingUpdates = false;
    wasError = false;
    errorDialog = NULL;

    quitButton = new QPushButton(this);
    quitButton->setIcon(QIcon(":/pics/application-exit.png"));
    quitButton->setText(tr("Quit"));
    ui->quitButtonBox->addButton(quitButton,QDialogButtonBox::AcceptRole);

    QSettings settings;
    interval = settings.value("settings/interval",60).toInt();
    err_interval = settings.value("settings/err_interval",5).toInt();
    doPlaySound = settings.value("settings/playSound",true).toBool();

    tray.setIcon(packageIcon);
    checkUpdatesAction = menu.addAction(QIcon(":/pics/updates.png"),tr("Check the updates"),this,SLOT(timeout()));
    updateAction = menu.addAction(QIcon(":/pics/PacmanTray-arch_logo.png"),tr("Update"),this,SLOT(onUpdate()));
    errorAction = menu.addAction(errorIcon,tr("Show the last errors..."),this,SLOT(onShowErrors()));
    menu.addAction(tr("Settings..."),this,SLOT(onSettings()));
    menu.addAction(tr("About..."),this,SLOT(onAbout()));
    menu.addAction(tr("Quit"),this,SLOT(onQuit()));
    tray.setContextMenu(&menu);
    connect(&tray,SIGNAL(activated(SystemTrayIcon::ActivationReason)),this,SLOT(trayActivated(SystemTrayIcon::ActivationReason)));

    connect(ui->toolBarWidget,SIGNAL(actionCheck_triggered()),checkUpdatesAction,SIGNAL(triggered()));
    connect(ui->toolBarWidget,SIGNAL(actionUpdate_triggered()),updateAction,SIGNAL(triggered()));
    connect(ui->toolBarWidget,SIGNAL(actionErrors_triggered()),errorAction,SIGNAL(triggered()));
    connect(this,SIGNAL(actionCheckUIUpdate(bool)),this,SLOT(onActionCheckUIUpdate(bool)));
    connect(this,SIGNAL(actionUpdateUIUpdate(bool)),this,SLOT(onActionUpdateUIUpdate(bool)));
    connect(this,SIGNAL(actionErrorsUIUpdate(bool)),this,SLOT(onActionErrorsUIUpdate(bool)));

    connect(&movie,SIGNAL(frameChanged(int)),this,SLOT(checking_frame()));
    connect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));

    movie.setFileName(":/pics/checking.gif");

    //QThread::sleep(4);
    timeout();
    setVisible(false);
    emit actionErrorsUIUpdate(false);
}

void MainWindow::onActionCheckUIUpdate(bool flag) {
    checkUpdatesAction->setEnabled(flag);
}

void MainWindow::onActionUpdateUIUpdate(bool flag) {
    updateAction->setEnabled(flag);
}

void MainWindow::onActionErrorsUIUpdate(bool flag) {
    errorAction->setEnabled(flag);
}

MainWindow::~MainWindow() {
    delete ui;
    checkingLock.unlock();
}

void MainWindow::on_quitButtonBox_accepted() {
    onQuit();
}

void MainWindow::on_buttonBox_accepted() {
    QSettings settings;
    interval = ui->intervalSpin->value();
    err_interval = ui->err_intervalSpin->value();
    doPlaySound = ui->playCheck->isChecked();
    settings.setValue("settings/interval",interval);
    settings.setValue("settings/err_interval",err_interval);
    settings.setValue("settings/playSound",doPlaySound);
    setVisible(false);
}

void MainWindow::on_buttonBox_rejected() {
    setVisible(false);
}

void MainWindow::trayActivated(SystemTrayIcon::ActivationReason reason) {
    if (reason == SystemTrayIcon::DoubleClick) onSettings();
    else if (reason == SystemTrayIcon::Trigger && !isCheckingUpdates) {
        if (packages.count() > 0) updateRequested();
        else if (isConnected) timeout();
    }
    else if (reason == SystemTrayIcon::ToolTip) {
        if (!wasError && !isCheckingUpdates) showPackagesBalloon();
        else if (wasError) showErrorsBalloon();
        else if (isCheckingUpdates) tray.showMessage(tr("Checking updates..."),"",SystemTrayIcon::Information,3000);
    }
}

void MainWindow::onUpdate() {
    QMetaObject::invokeMethod(this,"executePacman",Qt::QueuedConnection);
}

void MainWindow::executePacman() {
    if (!isGuiAppActive()) QProcess::startDetached(QString("%1/bin/QPacman --updates").arg(INSTALL_PREFIX));
}

void MainWindow::onSettings() {
    setVisible(true);
}

void MainWindow::onAbout() {
    QMessageBox::about(this,tr("About QPacmanTray..."),tr("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">QPacmanTray</span> is tray auto checking application for <span style=\" font-weight:600;\">QPacmanServer</span> and <span style=\" font-weight:600;\">QPacman</span>.</span></p>"
                                                          "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">QPacmanTray version is %1.</p>"
                                                          "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Developer: Alex Levkovich (<a href=\"mailto:alevkovich@tut.by\"><span style=\" text-decoration: underline; color:#0057ae;\">alevkovich@tut.by</span></a>)</p>"
                                                          "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">License: GPL</p>").arg(pacmantray_version));
}

bool MainWindow::isGuiAppActive() {
    QLockFile guiLock("/tmp/QPacman_client");
    return !guiLock.tryLock();
}

class TimeOutEnd {
public:
    TimeOutEnd(MainWindow * mainWindow,bool lastUpdate = false) {
        this->mainWindow = mainWindow;
        this->lastUpdate = lastUpdate;
    }

    ~TimeOutEnd() {
        if (lastUpdate) {
            mainWindow->movie.stop();
            mainWindow->isCheckingUpdates = false;
        }
        else {
            if (mainWindow->wasError) mainWindow->isCheckingUpdates = false;
        }
        emit mainWindow->actionCheckUIUpdate(!mainWindow->isGuiAppActive() && !mainWindow->isCheckingUpdates);
        emit mainWindow->actionUpdateUIUpdate(!mainWindow->isGuiAppActive() && !mainWindow->isCheckingUpdates && !mainWindow->wasError);
        emit mainWindow->actionErrorsUIUpdate(mainWindow->wasError && !mainWindow->isGuiAppActive());
        mainWindow->timer.start(((mainWindow->wasError)?mainWindow->err_interval:mainWindow->interval)*60000);
        if (mainWindow->wasError || lastUpdate) mainWindow->checkingLock.unlock();
    }

private:
    MainWindow * mainWindow;
    bool lastUpdate;
};

void MainWindow::timeout() {
    if (isCheckingUpdates) return;

    wasError = false;

    timer.stop();
    isCheckingUpdates = true;

    emit actionCheckUIUpdate(false);
    emit actionUpdateUIUpdate(false);

    TimeOutEnd temp(this);

    if (!checkingLock.tryLock()) {
        was_error(tr("Cannot lock checkingLock!!!"),"timeout");
        wasError = true;
        tray.setIcon(errorIcon);
        tray.show();
        return;
    }

    if (isGuiAppActive()) return;

    packages.clear();
    movie.start();
    tray.show();

    refresher = new PacmanDBRefresher(this);
    connect(refresher,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(db_refreshed(PacmanProcessReader *)));
    connect(refresher,SIGNAL(was_error(const QString &,const QString &)),this,SLOT(was_error(const QString &,const QString &)));
}

void MainWindow::checking_frame() {
    tray.setIcon(QIcon(movie.currentPixmap()));
}

void MainWindow::db_refreshed(PacmanProcessReader * reader) {
    int code = reader->exitCode();
    delete reader;
    refresher = NULL;

    TimeOutEnd temp(this,true);

    if (code != 0) return;

    PacmanSimpleUpdatesReader updatesreader;
    connect(&updatesreader,SIGNAL(was_error(const QString &,const QString &)),this,SLOT(was_error(const QString &,const QString &)));
    updatesreader.waitToComplete();
    if (updatesreader.exitCode() != 0) return;

    _areUpdates(updatesreader.packages());

    m_errors.clear();
    wasError = false;
    isCheckingUpdates = false;
}

void MainWindow::was_error(const QString & error,const QString & command) {
    if (error.isEmpty()) return;
    movie.stop();
    showErrorsBalloon();
    playSoundForWasError();
    m_errors[command] += error + '\n';
    wasError = true;
    tray.setIcon(errorIcon);
}

void MainWindow::_areUpdates(const QStringList & packages) {
    this->packages = packages;
    if (packages.count() > 0) {
        tray.setIcon(packageIcon);
        tray.setVisible(true);
        if (showPackagesBalloon()) playSoundForCheckOk();
    }
    else if (!wasError) tray.setVisible(false);
}

bool MainWindow::showPackagesBalloon() {
    if (packages.count() > 0) {
        tray.hideMessageBalloon();
        PacmanBalloonTip * balloon = new PacmanBalloonTip(packages,&tray);
        connect(balloon,SIGNAL(updateRequested()),this,SLOT(updateRequested()));
        balloon->show();
        return true;
    }
    return false;
}

void MainWindow::showErrorsBalloon() {
    if (wasError) {
        tray.hideMessageBalloon();
        PacmanErrorBalloonTip * balloon = new PacmanErrorBalloonTip(&tray);
        connect(balloon,SIGNAL(showErrorsRequested()),this,SLOT(showErrorsRequested()));
        balloon->show();
    }
}

void MainWindow::updateRequested() {
    tray.hideMessageBalloon();
    onUpdate();
}

void MainWindow::showErrorsRequested() {
    tray.hideMessageBalloon();
    onShowErrors();
}

void MainWindow::playSoundForCheckOk() {
    if (doPlaySound && (packages.count() > 0)) {
        good_player.play();
    }
}

void MainWindow::playSoundForWasError() {
    if (doPlaySound) {
        bad_player.play();
    }
}

void MainWindow::onQuit() {
    if (isCheckingUpdates) {
        if (QMessageBox::warning(this,tr("Warning..."),tr("Update checking is in progress!\nQuiting will abort this operation.\nIt is not good idea to abort pacman execution, Continue?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::No) return;
    }
    qApp->quit();
}

void MainWindow::onShowErrors() {
    if (errorDialog != NULL) delete errorDialog;

    QString error;

    QMapIterator<QString,QString> i(m_errors);
    while (i.hasNext()) {
        i.next();
        error += tr("Command: %1:\n").arg(i.key()) + i.value() + "\n\n";
    }

    errorDialog = new ErrorDialog(tr("Press button to see the errors."),error,this);
    errorDialog->exec();
    delete errorDialog;
    errorDialog = NULL;
}

void MainWindow::showEvent(QShowEvent * event) {
    QMainWindow::showEvent(event);

    adjustSize();
    ui->quitButtonBox->setMaximumWidth(quitButton->width());
    adjustSize();

    QSettings settings;
    interval = settings.value("settings/interval",60).toInt();
    err_interval = settings.value("settings/err_interval",5).toInt();
    doPlaySound = settings.value("settings/playSound",true).toBool();

    ui->intervalSpin->setValue(interval);
    ui->err_intervalSpin->setValue(err_interval);
    ui->playCheck->setChecked(doPlaySound);
}

void MainWindow::onGuiExited() {
    emit actionCheckUIUpdate(!isCheckingUpdates);
    emit actionUpdateUIUpdate(!isCheckingUpdates);
}

void MainWindow::onGuiStarted() {
    emit actionCheckUIUpdate(false);
    emit actionUpdateUIUpdate(false);
}

void MainWindow::terminate() {
    if (refresher != NULL) refresher->terminate();
}

void MainWindow::dbusLoaded() {
    isConnected = true;
    wasError = false;
    if (packages.count() > 0) tray.setIcon(packageIcon);
    else tray.hide();
    timeout();
}

void MainWindow::dbusUnloaded() {
    isConnected = false;
    checkingLock.unlock();
    isCheckingUpdates = false;
    was_error(tr("QPacmanServer service is unloaded!!! Restart it."),"DBus");
    timer.stop();
    tray.show();
    emit actionCheckUIUpdate(false);
    emit actionUpdateUIUpdate(false);
    emit actionErrorsUIUpdate(wasError && !isGuiAppActive());
}
