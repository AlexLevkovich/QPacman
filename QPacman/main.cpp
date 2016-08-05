/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "mainwindow.h"
#include "localpackagemainwindow.h"
#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QTranslator>
#include "pacmanprocessreader.h"
#include "pacmanserverinterface.h"
#include "pacmaninstallpackagesreader.h"
#include "dbuswatcher.h"
#include "static.h"
#include "qlockfile.h"

const char * pacman_version = "2.2";

class ClientStarter {
public:
    ClientStarter() {
        PacmanServerInterface::instance()->commandRequest("CLIENT STARTED");
    }
    ~ClientStarter() {
        PacmanServerInterface::instance()->commandRequest("CLIENT EXITED");
    }
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QTranslator m_translator;
    QString lang = QLocale::system().name().split("_").at(0);
    if(!m_translator.load("qpacman_" + lang, TRANS_DIR2))
        m_translator.load("qpacman_" + lang, TRANS_DIR1);
    QApplication::installTranslator(&m_translator);

    QTranslator m_translator2;
    if (m_translator2.load(QLocale::system(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&m_translator2);
    }

    if (Static::userName() == "root") {
        QMessageBox::critical(NULL,Static::Error_Str,QObject::tr("You should not be root!"),QMessageBox::Ok);
        return 1;
    }

    Static::init_tr_variables();

    QLockFile sharedLock(QString("%1/QPacman_client").arg(QDir::tempPath()));
    if(!sharedLock.tryLock()) return 1;

    QLockFile traySharedLock(QString("%1/QPacmanTray_checker").arg(QDir::tempPath()));
    if(!traySharedLock.tryLock()) {
        QMessageBox::critical(NULL,Static::Error_Str,QObject::tr("QServerTray is checking for the updates...\nPlease wait some time."),QMessageBox::Ok);
        return 1;
    }
    traySharedLock.unlock();

    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("QPacman");
    qRegisterMetaType<PacmanProcessReader *>("PacmanProcessReader *");
    qRegisterMetaType<PacmanInstallPackagesReader *>("PacmanInstallPackagesReader *");

    PacmanServerInterface::createInstance(&a);
    if (!PacmanServerInterface::instance()->isValid()) {
        QMessageBox::critical(NULL,Static::Error_Str,QObject::tr("Cannot connect to dbus' QPacmanServer!"),QMessageBox::Ok);
        return 1;
    }

    ClientStarter starter;

    int ret = 127;

    QStringList packages;
    if (argc >= 2) {
        for (int i=1;i<argc;i++) {
            if (QFileInfo(argv[i]).exists()) packages.append(argv[i]);
        }
    }

    if (packages.count() > 0) {
        try {
            LocalPackageMainWindow w(packages);
            QObject::connect(PacmanServerInterface::watcher(),SIGNAL(loaded()),&w,SLOT(enableActions()));
            QObject::connect(PacmanServerInterface::watcher(),SIGNAL(unloaded()),&w,SLOT(disableActions()));
            w.show();
            ret = a.exec();
        }
        catch (QString error) {
            qCritical() << error;
            exit(127);
        }
    }
    else {
        try {
            MainWindow w((argc >= 2) && (QString(argv[1]) == "--updates"));
            QObject::connect(PacmanServerInterface::watcher(),SIGNAL(loaded()),&w,SLOT(dbus_loaded()));
            QObject::connect(PacmanServerInterface::watcher(),SIGNAL(unloaded()),&w,SLOT(dbus_unloaded()));
            w.show();
            ret = a.exec();
        }
        catch (QString error) {
            qCritical() << error;
            exit(127);
        }
    }

    return ret;
}
