/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include <QCoreApplication>
#include "singleapplication.h"
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <shadow.h>
#include <QSettings>
#include <QStringList>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QLibraryInfo>

QString pacman_cache_dir;
QString pacman_conf;
QString pacman_lock_file;
QString pacman_db_path;
QString invalid_password_str;

const QString userName() {
    struct passwd *pw;

    pw = getpwuid(geteuid());
    return QString::fromLocal8Bit((pw != NULL)?pw->pw_name:"");
}

const QByteArray encryptedUserPassword() {
    struct passwd *pw;
    struct spwd *passw;

    pw = getpwuid(geteuid());
    if (pw == NULL) return QByteArray();

    passw = getspnam(pw->pw_name);
    if (passw == NULL) return QByteArray();

    QByteArray arr(passw->sp_pwdp);
    int index = arr.lastIndexOf("$");
    if (index == -1) return QByteArray();

    return arr.mid(index+1);
}

const QByteArray encryptedSalt() {
    struct passwd *pw;
    struct spwd *passw;

    pw = getpwuid(geteuid());
    if (pw == NULL) return QByteArray();

    passw = getspnam(pw->pw_name);
    if (passw == NULL) return QByteArray();

    QByteArray arr(passw->sp_pwdp);
    int index = arr.lastIndexOf("$");
    if (index == -1) return QByteArray();

    return arr.left(index);
}

const QString arch() {
    utsname uts;
    if (::uname(&uts)) {
        fprintf(stderr,"Cannot return the system ARCH!!!");
        ::exit(127);
        return QString();
    }

    return QString(uts.machine);
}

const QStringList repo_db_urls() {
    QSettings settings(pacman_conf,QSettings::IniFormat);
    QStringList groups = settings.childGroups();
    for (int i=(groups.count()-1);i>=0;i--) {
        if (groups[i] == "options") groups.removeAt(i);
    }

    QStringList urls;
    for (int i=0;i<groups.count();i++) {
        QString server = settings.value(QString("%1/Server").arg(groups[i]),"").toString();
        if (server.isEmpty()) {
            QString include = settings.value(QString("%1/Include").arg(groups[i]),"").toString();
            if (include.isEmpty()) continue;
            QSettings settings2(include,QSettings::IniFormat);
            server = settings2.value("Server","").toString();
            if (server.isEmpty()) continue;
        }
        server.replace("$repo",groups[i]);
        server.replace("$arch",arch());
        server+="/"+groups[i]+".db.tar.gz";
        urls << server;
    }

    return urls;
}

int main(int argc, char *argv[]) {
    SingleApplication a(argc, argv);

    QTranslator m_translator;
    QString lang = QLocale::system().name().split("_").at(0);
    if(!m_translator.load("qpacmanservice_" + lang, TRANS_DIR2))
        m_translator.load("qpacmanservice_" + lang, TRANS_DIR1);
    QCoreApplication::installTranslator(&m_translator);

    QTranslator m_translator2;
    if (m_translator2.load(QLocale::system(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        QCoreApplication::installTranslator(&m_translator2);
    }

    invalid_password_str = QObject::tr("Invalid password!!!");
    if (userName() != "root") {
        qCritical() << QObject::tr("This program should be run under root!!!");
        return 1;
    }

    qRegisterMetaType<QList<int> >("QList<int>");

    return a.exec();
}
