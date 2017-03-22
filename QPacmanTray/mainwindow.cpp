/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QPixmap>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include "pacmandbrefresher.h"
#include "pacmansimpleupdatesreader.h"
#include "installprogressloop.h"
#include "messagedialog.h"
#include <QMessageBox>
#include <QPushButton>
#include "errordialog.h"
#include <QDebug>
#include "suchecker.h"
#include "rootdialog.h"
#include "static.h"
#include <cpuid.h>

extern const char * pacmantray_version;
const char * error_str = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                         "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" font-style:normal;\">"
                         "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">%1</p>"
                         "</body></html>";
static const char * message_str_part1 = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                                        "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" font-style:normal;\">"
                                        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">%1</span></p>";
static const char * message_str_packg = "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">%1</p>";
static const char * message_str_part2 = "</body></html>";


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow),
                                          errorIcon(":/pics/kpk-important.png"),
                                          packageIcon(":/pics/package-update.png"),
                                          good_player(":/sound/KDE-Sys-App-Positive.ogg"),
                                          bad_player(":/sound/KDE-Sys-App-Error.ogg"),
                                          checkingLock(QDir::tempPath()+QDir::separator()+"QPacmanTray_checker"),
                                          tray(NULL) {
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

    checkUpdatesAction = new QAction(QIcon(":/pics/updates.png"),tr("Check the updates"),this);
    updateAction = new QAction(QIcon(":/pics/fullupdate.png"),tr("Update"),this);
    errorAction = new QAction(errorIcon,tr("Show the last errors..."),this);
    qpacmanAction = new QAction(QIcon(":/pics/Pacman-arch_logo.png"),tr("Show updates in QPacman..."),this);

    connect(&tray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));


    connect(checkUpdatesAction,SIGNAL(triggered()),this,SLOT(timeout()));
    connect(updateAction,SIGNAL(triggered()),this,SLOT(updateRequested()));
    connect(qpacmanAction,SIGNAL(triggered()),this,SLOT(onLoadQPacman()));
    connect(errorAction,SIGNAL(triggered()),this,SLOT(onShowErrors()));

    connect(ui->toolBarWidget,SIGNAL(actionCheck_triggered()),checkUpdatesAction,SIGNAL(triggered()));
    connect(ui->toolBarWidget,SIGNAL(actionUpdate_triggered()),updateAction,SIGNAL(triggered()));
    connect(ui->toolBarWidget,SIGNAL(actionErrors_triggered()),errorAction,SIGNAL(triggered()));
    connect(this,SIGNAL(actionCheckUIUpdate(bool)),this,SLOT(onActionCheckUIUpdate(bool)));
    connect(this,SIGNAL(actionUpdateUIUpdate(bool)),this,SLOT(onActionUpdateUIUpdate(bool)));
    connect(this,SIGNAL(actionErrorsUIUpdate(bool)),this,SLOT(onActionErrorsUIUpdate(bool)));

    connect(&movie,SIGNAL(frameChanged(int)),this,SLOT(checking_frame()));
    connect(&wait_movie,SIGNAL(frameChanged(int)),this,SLOT(checking_frame()));
    connect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));

    movie.setFileName(":/pics/checking.gif");
    movie.setSpeed(60);
    wait_movie.setFileName(":/pics/waiting.gif");

    timeout();
    setVisible(false);
    emit actionErrorsUIUpdate(false);
}

void MainWindow::showTray(const QIcon & icon) {
    if (!tray.isVisible()) {
        QMenu * tray_menu = new QMenu(this);
        tray_menu->addAction(checkUpdatesAction);
        tray_menu->addAction(updateAction);
        tray_menu->addAction(qpacmanAction);
        tray_menu->addAction(errorAction);
        tray_menu->addAction(tr("Settings..."),this,SLOT(onSettings()));
        tray_menu->addAction(tr("About..."),this,SLOT(onAbout()));
        tray_menu->addAction(tr("Quit"),this,SLOT(onQuit()));

        tray.setContextMenu(tray_menu);
    }
    tray.setIcon(icon);
    if (!tray.isVisible()) tray.show();
}

void MainWindow::hideTray() {
    QMenu * tray_menu = tray.contextMenu();
    if (tray_menu != NULL) tray_menu->deleteLater();
    tray.hide();
}

void MainWindow::onActionCheckUIUpdate(bool flag) {
    checkUpdatesAction->setEnabled(flag);
}

void MainWindow::onActionUpdateUIUpdate(bool flag) {
    updateAction->setEnabled(flag);
    qpacmanAction->setEnabled(flag);
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

void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) onSettings();
    else if (reason == QSystemTrayIcon::Trigger && !isCheckingUpdates) {
        if (packages.count() > 0) updateRequested();
        else if (isConnected) timeout();
    }
}

void MainWindow::startFullUpdate() {
    emit actionCheckUIUpdate(false);
    emit actionUpdateUIUpdate(false);
    emit actionErrorsUIUpdate(false);

    if (!Static::checkRootAccess()) {
        QMessageBox::critical(this,Static::Error_Str,Static::RootRightsNeeded_Str,QMessageBox::Ok);
    }
    else {
        checkingLock.unlock();
        if (!checkingLock.tryLock()) {
            was_error(tr("Cannot lock checkingLock!!!"),"timeout");
            return;
        }

        InstallProgressLoop iprogress_dlg(Static::su_password,this);
        connect(&iprogress_dlg,SIGNAL(post_messages(const QString &,const QStringList &)),this,SLOT(add_post_messages(const QString &,const QStringList &)));
        connect(&iprogress_dlg,SIGNAL(end_waiting_mode()),this,SLOT(stop_wait_animation()));
        connect(&iprogress_dlg,SIGNAL(start_waiting_mode()),this,SLOT(start_wait_animation()));
        if (iprogress_dlg.exec() == QDialog::Accepted) {
            MessageDialog::post(Static::InstalledSuccess_Str,m_messages,tr("Post messages..."));
        }
        checkingLock.unlock();
    }

    onGuiExited();
    m_messages.clear();
}

void MainWindow::add_post_messages(const QString & package,const QStringList & messages) {
    m_messages += Static::PostMessages_Str.arg(package)+"\n\n";
    m_messages += messages.join("\n");
    m_messages += '\n';
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
    QLockFile guiLock(QDir::tempPath()+QDir::separator()+"QPacman_client");
    return !guiLock.tryLock();
}

class TimeOutEnd {
public:
    TimeOutEnd(MainWindow * mainWindow,bool lastUpdate = false) {
        this->mainWindow = mainWindow;
        this->lastUpdate = lastUpdate;
        def_checking_updates = false;
    }

    ~TimeOutEnd() {
        mainWindow->isCheckingUpdates = def_checking_updates;
        if (lastUpdate) mainWindow->movie.stop();
        emit mainWindow->actionCheckUIUpdate(!mainWindow->isGuiAppActive() && !mainWindow->isCheckingUpdates);
        emit mainWindow->actionUpdateUIUpdate(!mainWindow->isGuiAppActive() && !mainWindow->isCheckingUpdates && !mainWindow->wasError);
        emit mainWindow->actionErrorsUIUpdate(mainWindow->wasError && !mainWindow->isGuiAppActive());
        mainWindow->timer.start(((mainWindow->wasError)?mainWindow->err_interval:mainWindow->interval)*60000);
        if (mainWindow->wasError || lastUpdate) mainWindow->checkingLock.unlock();
    }
	
public:
    bool def_checking_updates;

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
        return;
    }

    if (isGuiAppActive()) {
        wasError = true;
        return;
    }

	temp.def_checking_updates = true;
	packages.clear();
    movie.start();
    showTray(errorIcon);
    tray.setToolTip(tr("Checking updates..."));

    refresher = new PacmanDBRefresher("",this);
    connect(refresher,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(db_refreshed(PacmanProcessReader *)));
}

void MainWindow::checking_frame() {
    QMovie * p_movie = (QMovie *)this->sender();
    showTray(QIcon(p_movie->currentPixmap()));
}

void MainWindow::db_refreshed(PacmanProcessReader * reader) {
    if (reader != NULL && reader->exitCode() != 0) was_error(reader->errorStream(),tr("Refreshing pacman database"));

    TimeOutEnd temp(this,reader != NULL);

    if (reader != NULL) {
        int code = reader->exitCode();
        delete reader;
        refresher = NULL;
        if (code != 0) return;
    }

    PacmanSimpleUpdatesReader updatesreader;
    updatesreader.waitToComplete();
    if (updatesreader.exitCode() != 0) {
        was_error(updatesreader.errorStream(),tr("Reading the list of updated packages"));
        return;
    }

    _areUpdates(updatesreader.packages());

    m_errors.clear();
    wasError = false;
    isCheckingUpdates = false;
}

void MainWindow::was_error(const QString & error,const QString & command) {
    if (error.isEmpty()) return;
    movie.stop();
    m_errors[command] += error + '\n';
    wasError = true;
    showTray(errorIcon);
    tray.setToolTip(QString(error_str).arg(tr("There were the errors during the last checking...")));
    playSoundForWasError();
}

void MainWindow::_areUpdates(const QStringList & packages) {
    this->packages = packages;
    if (packages.count() > 0) {
        showTray(packageIcon);

        QString message_str = QString(message_str_part1).arg(tr("New packages are available:"));
        int count = qMin(packages.count(),6);
        for (int i=0;i<count;i++) {
            message_str+=QString(message_str_packg).arg(packages[i]);
        }
        if (packages.count() > 6) message_str+=QString(message_str_packg).arg("...");
        message_str+=message_str_part2;
        tray.setToolTip(message_str);

        playSoundForCheckOk();
    }
    else if (!wasError) hideTray();
}

void MainWindow::updateRequested() {
    if (updateAction->isEnabled()) QMetaObject::invokeMethod(this,"startFullUpdate",Qt::QueuedConnection);
}

void MainWindow::showErrorsRequested() {
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
    timer.stop();
    db_refreshed(NULL);
}

void MainWindow::onGuiStarted() {
    emit actionCheckUIUpdate(false);
    emit actionUpdateUIUpdate(false);
    hideTray();
}

void MainWindow::terminate() {
    if (refresher != NULL) refresher->terminate();
}

void MainWindow::onLoadQPacman() {
    if (!isGuiAppActive()) QProcess::startDetached(QString("%1/bin/QPacman --updates").arg(INSTALL_PREFIX));
}

void MainWindow::start_wait_animation() {
    wait_movie.start();
}

void MainWindow::stop_wait_animation() {
    wait_movie.stop();
    showTray(packageIcon);
}
