/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include "messagedialog.h"
#include <QTranslator>
#include <QLibraryInfo>
#include "static.h"
#include "qpacmanapplication.h"

int main(int argc, char *argv[]) {
    if (getuid() != 0 && (argc <= 1 || (strcmp(argv[1],"--user") || (argc != 2)))) {
        return Static::startUnderRoot(argc,argv);
    }

    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("QPacman");

    QPacmanApplication * a = new QPacmanApplication(argc, argv);

    Static::setupTranslations(QString::fromLatin1("qpacman"),QDir(TRANS_DIR2),QDir(TRANS_DIR1),QDir(TRANS_DIR4),QDir(TRANS_DIR3));
    Static st;

    Alpm alpm;
    if (!alpm.open(PACMANCONF)) {
        qCritical() << "QPacmanApplication: Alpm hasn't been initialized!!!";
        return 1;
    }

    int ret = a->exec();
    delete a;
    return ret;
}
