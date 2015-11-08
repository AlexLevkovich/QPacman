/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "mainwindow.h"
#include "singleapplication.h"
#include "pacmanprocessreader.h"
#include <QTranslator>

const char * pacmantray_version = "2.2";

int main(int argc, char *argv[]) {
    SingleApplication a(argc, argv);

    QTranslator m_translator;
    QString lang = QLocale::system().name().split("_").at(0);
    if(!m_translator.load("qpacmantray_" + lang, TRANS_DIR2))
        m_translator.load("qpacmantray_" + lang, TRANS_DIR1);
    QApplication::installTranslator(&m_translator);

    QTranslator m_translator2;
    if (m_translator2.load(QLocale::system(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&m_translator2);
    }

    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("QPacmanTray");
    qRegisterMetaType<PacmanProcessReader *>("PacmanProcessReader *");

    if (a.isStarted()) {
        MainWindow w;
        a.setMainWidget(&w);
        return a.exec();
    }

    return 0;
}
