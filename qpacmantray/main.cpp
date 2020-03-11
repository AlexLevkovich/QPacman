/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "qpacmantrayapplication.h"
#include <QDir>
#include <QDebug>
#include "libalpm.h"
#include "static.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("QPacmanTray");

    QPacmanTrayApplication app(argc,argv);

    Static::setupTranslations(QString::fromLatin1("qpacmantray"),QDir(TRANS_DIR2),QDir(TRANS_DIR1),QDir(TRANS_DIR4),QDir(TRANS_DIR3));
    Static st;

    Alpm alpm;
    if (!alpm.open(PACMANCONF,QFileInfo(QSettings().fileName()).canonicalPath())) {
        qCritical() << QString::fromLatin1("Alpm hasn't been initialized!");
        return 1;
    }

    return app.exec();
}
