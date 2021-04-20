#include "qpacmantrayapplication.h"
#include "libalpm.h"
#include "static.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("QPacmanTray");

    QPacmanTrayApplication a(argc, argv);
    if (a.isSecondary()) return a.exec();

    setupTranslations(QString::fromLatin1("qpacmantray"),TRANS_DIR2,TRANS_DIR1,TRANS_DIR4,TRANS_DIR3);

    Alpm alpm;
    return a.exec();
}
