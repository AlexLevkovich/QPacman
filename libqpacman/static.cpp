/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "static.h"
#include "rootdialog.h"
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#include <QApplication>
#include "suchecker.h"
#include "pacmanentry.h"
#include <QRegExp>
#include <QList>
#include <QUrl>
#include <QDebug>
#include <QDesktopWidget>
#include "windowcenterer.h"

QString Static::su_password;
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
    if (!su_password.isEmpty()) {
        SuChecker suchecker(su_password);
        suchecker.waitToComplete();
        if (suchecker.ok()) return true;
    }

    for (int i=0;i<3;i++) {
        RootDialog dlg;
        if (dlg.exec() == QDialog::Rejected) break;

        SuChecker suchecker(dlg.password());
        suchecker.waitToComplete();
        if (!suchecker.ok()) continue;

        su_password = dlg.password();
        return true;
    }

    return false;
}



const QString Static::arch() {
    utsname name;
    uname(&name);
    return QString(name.machine);
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

const QList<QAction *> Static::childrenActions(QObject * main_object) {
    QList<QAction *> actions;
    QObjectList listObjects = main_object->children();
    for (int i=0;i<listObjects.count();i++) {
        actions.append(childrenActions(listObjects[i]));
        if (listObjects[i]->inherits("QAction")) actions.append((QAction *)listObjects[i]);
    }

    return actions;
}

const QString Static::htmlFragmentToText(const QTextDocumentFragment & fragment) {
    struct TextMarker {
        qint64 begin_index;
        qint64 count;
        QString replace_to;
    } marker;

    QString html = fragment.toHtml();
    QRegExp imgTagRegex("\\<img[^\\>]*src\\s*=\\s*\"([^\"]*)\"[^\\>]*\\>", Qt::CaseInsensitive);
    imgTagRegex.setMinimal(true);
    QList<TextMarker> markers;
    QUrl url;
    marker.begin_index = 0;
    while((marker.begin_index = imgTagRegex.indexIn(html, marker.begin_index)) != -1) {
        marker.count = imgTagRegex.matchedLength();
        url.setUrl(imgTagRegex.cap(1));
        if (url.scheme() == "qpc") {
            marker.replace_to = PacmanEntry::urlParmsToPacmanDep(url.path().mid(1)+"?"+url.query());
            markers.append(marker);
        }
        marker.begin_index += marker.count;
    }

    for (int i=(markers.count()-1);i>=0;i--) {
        html.replace(markers[i].begin_index,markers[i].count,markers[i].replace_to);
    }

    return QTextDocumentFragment::fromHtml(html).toPlainText();
}

int Static::isPackageInstalled(const QString & name) {
    QProcess pacman;
    pacman.start(QString("%1 -Qi %2").arg(PACMAN_BIN).arg(name));
    pacman.waitForFinished(-1);
    return pacman.exitCode();
}

void Static::makeCentered(QWidget * wnd) {
    if (wnd == NULL) return;

    new WindowCenterer(wnd);
}
