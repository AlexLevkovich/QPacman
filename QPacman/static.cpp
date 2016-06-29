/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "static.h"
#include "pacmansaltreader.h"
#include "rootdialog.h"
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#include <QApplication>
#include "pacmanprovidersdialog.h"

QByteArray Static::encryptedPassword;
bool Static::isRootAccessOK = false;
QString Static::Error_Str;
QString Static::Warning_Str;
QString Static::Question_Str;
QString Static::Info_Str;
QString Static::InstalledSuccess_Str;
QString Static::PostMessages_Str;
QString Static::RootRightsNeeded_Str;
QString Static::PacmanTerminate_Str;
QString Static::RepoAll_Str;

void Static::init_tr_variables() {
    RepoAll_Str = QObject::tr("All");
    Error_Str = QObject::tr("Error...");
    Warning_Str = QObject::tr("Warning...");
    Info_Str = QObject::tr("Information...");
    Question_Str = QObject::tr("Question...");
    InstalledSuccess_Str = QObject::tr("The packages were installed successfully!");
    PostMessages_Str = QObject::tr("Post messages for %1 package...");
    RootRightsNeeded_Str = QObject::tr("The root's rights are needed to continue!!!");
    PacmanTerminate_Str = QObject::tr("It is not good idea to terminate the pacman's execution.\nAre you sure?");
}

bool Static::checkRootAccess() {
    if (isRootAccessOK) return true;

    PacmanSaltReader salt_reader;
    salt_reader.waitToComplete();
    if (salt_reader.exitCode() != 0) return false;

    for (int i=0;i<3;i++) {
        RootDialog dlg(salt_reader.salt());
        if (dlg.exec() == QDialog::Rejected) break;
        if (!dlg.passwordIsOK()) continue;

        isRootAccessOK = true;
        return true;
    }

    return false;
}



const QString Static::arch() {
    utsname name;
    uname(&name);
    return QString(name.machine);
}

const QByteArray Static::encryptUserPassword(const char * password,const char * salt) {
    QByteArray arr(crypt(password,salt));

    int index = arr.lastIndexOf("$");
    if (index == -1) return QByteArray();

    encryptedPassword = arr.mid(index+1);
    return encryptedPassword;
}

QMainWindow * Static::findMainWindow() {
    foreach(QWidget *widget, qApp->topLevelWidgets()) {
        if(widget->inherits("QMainWindow")) {
            return (QMainWindow *)widget;
        }
    }

    return NULL;
}

const QString Static::userName() {
    struct passwd *pw;

    pw = getpwuid(geteuid());
    return QString::fromLocal8Bit((pw != NULL)?pw->pw_name:"");
}

const QString Static::someProvidersAvailable(QWidget * parent,const QStringList & providers) {
    PacmanProvidersDialog dlg(providers,parent);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.provider();
    }
    return QString();
}

const QList<QAction *> Static::childrenActions(QObject * main_object) {
    QList<QAction *> actions;
    QObjectList listObjects = main_object->children();
    for (int i=0;i<listObjects.count();i++) {
        actions.append(childrenActions(listObjects[i]));
        if (listObjects[i]->inherits("QAction")) actions.append((QAction *)listObjects[i]);
    }

    return actions;
}
