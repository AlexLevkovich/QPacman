/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "suapplication.h"
#include <QDir>
#include "static.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("QSu");

    SuApplication app(argc,argv);
    Static::setupTranslations(QString::fromLatin1("qsu"),QDir(TRANS_DIR2),QDir(TRANS_DIR1),QDir(TRANS_DIR4),QDir(TRANS_DIR3));

    return app.exec();
}
