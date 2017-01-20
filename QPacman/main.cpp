/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "mainwindow.h"
#include "localpackagemainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>
#include <QLibraryInfo>
#include "pacmanprocessreader.h"
#include "pacmaninstallpackagesreader.h"
#include "static.h"
#if QT_VERSION >= 0x050000
#include <QLockFile>
#else
#include "qlockfile.h"
#endif

const char * pacman_version = "2.4";


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
