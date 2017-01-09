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
#include "pacmandbrefresher.h"
#include "pacmansimpleupdatesreader.h"
#include <QMessageBox>
#include <QPushButton>
#include "errordialog.h"
#include <QDebug>
#include "suchecker.h"
#include "rootdialog.h"
#include "simplecrypt.h"
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

class Cryptor {
public:
    static QString decrypt(const QString &password) {
        return SimpleCrypt(getCpuHash()).decryptToString(password);
    }

    static QString encrypt(const QString &password) {
        return SimpleCrypt(getCpuHash()).encryptToString(password);
    }

private:
    static unsigned short getCpuHash() {
        unsigned int level = 0;
        unsigned int cpuinfo[4] = { 0, 0, 0, 0 };

        __get_cpuid(level, &cpuinfo[0], &cpuinfo[1], &cpuinfo[2], &cpuinfo[3]);

       unsigned short hash = 0;
       unsigned int* ptr = (&cpuinfo[0]);
       for ( unsigned int i = 0; i < 4; i++ )
          hash += (ptr[i] & 0xFFFF) + ( ptr[i] >> 16 );

       return hash;
    }
};

static const QString su_password() {
    QSettings settings;
    return Cryptor::decrypt(settings.value("settings/su_password","").toString());
}

static void setSuPassword(const QString & su_password) {
    QSettings settings;
    settings.setValue("settings/su_password",Cryptor::encrypt(su_password));
}

static bool checkRootAccess() {
    if (!su_password().isEmpty()) {
        SuChecker suchecker(su_password());
        suchecker.waitToComplete();
        if (suchecker.ok()) return true;
    }

    for (int i=0;i<3;i++) {
        RootDialog dlg;
        if (dlg.exec() == QDialog::Rejected) break;

        SuChecker suchecker(dlg.password());
        suchecker.waitToComplete();
        if (!suchecker.ok()) continue;

        setSuPassword(dlg.password());
        return true;
    }

    return false;
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow),
                                          errorIcon(":/pics/kpk-important.png"),
                                          packageIcon(":/pics/distro-upgrade.png"),
                                          good_player(":/sound/KDE-Sys-App-Positive.ogg"),
                                          bad_player(":/sound/KDE-Sys-App-Error.ogg"),
                                          checkingLock("/tmp/QPacmanTray_checker"),
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
    updateAction = new QAction(QIcon(":/pics/PacmanTray-arch_logo.png"),tr("Update"),this);
    errorAction = new QAction(errorIcon,tr("Show the last errors..."),this);
    connect(checkUpdatesAction,SIGNAL(triggered()),this,SLOT(timeout()));
    connect(updateAction,SIGNAL(triggered()),this,SLOT(onUpdate()));
    connect(errorAction,SIGNAL(triggered()),this,SLOT(onShowErrors()));

    connect(ui->toolBarWidget,SIGNAL(actionCheck_triggered()),checkUpdatesAction,SIGNAL(triggered()));
    connect(ui->toolBarWidget,SIGNAL(actionUpdate_triggered()),updateAction,SIGNAL(triggered()));
    connect(ui->toolBarWidget,SIGNAL(actionErrors_triggered()),errorAction,SIGNAL(triggered()));
    connect(this,SIGNAL(actionCheckUIUpdate(bool)),this,SLOT(onActionCheckUIUpdate(bool)));
    connect(this,SIGNAL(actionUpdateUIUpdate(bool)),this,SLOT(onActionUpdateUIUpdate(bool)));
    connect(this,SIGNAL(actionErrorsUIUpdate(bool)),this,SLOT(onActionErrorsUIUpdate(bool)));

    connect(&movie,SIGNAL(frameChanged(int)),this,SLOT(checking_frame()));
    connect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));

    movie.setFileName(":/pics/checking.gif");
    movie.setSpeed(60);

    timeout();
    setVisible(false);
    emit actionErrorsUIUpdate(false);
}

void MainWindow::showTray(const QIcon & icon) {
    if (tray == NULL) {
        tray = new QSystemTrayIcon(this);
        QMenu * menu = new QMenu(this);
        menu->addAction(checkUpdatesAction);
        menu->addAction(updateAction);
        menu->addAction(errorAction);
        menu->addAction(tr("Settings..."),this,SLOT(onSettings()));
        menu->addAction(tr("About..."),this,SLOT(onAbout()));
        menu->addAction(tr("Quit"),this,SLOT(onQuit()));
        tray->setContextMenu(menu);
        connect(tray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
    }
    tray->setIcon(icon);
    tray->show();
}

void MainWindow::hideTray() {
    if (tray == NULL) return;
    QMenu * menu = tray->contextMenu();
    delete tray;
    if (menu != NULL) delete menu;
    tray = NULL;
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

void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) onSettings();
    else if (reason == QSystemTrayIcon::Trigger && !isCheckingUpdates) {
        if (packages.count() > 0) updateRequested();
        else if (isConnected) timeout();
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
    tray->setToolTip(tr("Checking updates..."));

    if (!checkRootAccess()) {
        was_error(tr("The root's rights are needed to continue!!!"),"timeout");
        return;
    }

    refresher = new PacmanDBRefresher(su_password(),this);
    connect(refresher,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(db_refreshed(PacmanProcessReader *)));
    connect(refresher,SIGNAL(was_error(const QString &,const QString &)),this,SLOT(was_error(const QString &,const QString &)));
}

void MainWindow::checking_frame() {
    showTray(QIcon(movie.currentPixmap()));
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
    m_errors[command] += error + '\n';
    wasError = true;
    showTray(errorIcon);
    tray->setToolTip(QString(error_str).arg(tr("There were the errors during the last checking...")));
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
        tray->setToolTip(message_str);

        playSoundForCheckOk();
    }
    else if (!wasError) hideTray();
}

void MainWindow::updateRequested() {
    onUpdate();
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
    emit actionCheckUIUpdate(!isCheckingUpdates);
    emit actionUpdateUIUpdate(!isCheckingUpdates);
    timeout();
}

void MainWindow::onGuiStarted() {
    emit actionCheckUIUpdate(false);
    emit actionUpdateUIUpdate(false);
    hideTray();
}

void MainWindow::terminate() {
    if (refresher != NULL) refresher->terminate();
}

