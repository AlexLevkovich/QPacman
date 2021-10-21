#include "mainwindow.h"
#include "singleapplication.h"
#include "libalpm.h"
#include "static.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("QPacman");
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    SingleApplication a(argc, argv,false,SingleApplication::System);

    setupTranslations(QString::fromLatin1("qpacman"),TRANS_DIR2,TRANS_DIR1,TRANS_DIR4,TRANS_DIR3);

    Alpm alpm;
    MainWindow w;
    w.show();
    return a.exec();
}
