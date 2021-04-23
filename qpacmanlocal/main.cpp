/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "localpackagemainwindow.h"
#include "singleapplication.h"
#include "static.h"
#include "libalpm.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("QPacmanLocal");

    SingleApplication a(argc, argv);

    setupTranslations(QString::fromLatin1("qpacmanlocal"),TRANS_DIR2,TRANS_DIR1,TRANS_DIR4,TRANS_DIR3);

    Alpm alpm;
    QStringList args = QCoreApplication::arguments();
    LocalPackageMainWindow w((args.count()>1)?args.mid(1):QStringList());
    w.show();
    return a.exec();
}
