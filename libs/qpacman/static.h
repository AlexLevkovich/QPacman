/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef STATIC_H
#define STATIC_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QSettings>
#include <QAction>
#include "singleapplication.h"
#include <QDir>

class SingleApplication;

class Static {
public:
    Static();

    static QString Error_Str;
    static QString Warning_Str;
    static QString Question_Str;
    static QString Info_Str;
    static QString InstalledSuccess_Str;
    static QString PostMessages_Str;
    static QString TransTerminate_Str;
    static QString RepoAll_Str;
    static QString QPacman_Key;

    static int startUnderRoot(int argc, char *argv[]);
    // returns pid or -1
    static bool startQPacman(const QStringList & args = QStringList(),QObject * receiver = NULL,const char * finished_slot = NULL,const QString & output_file_name = QString());
    static SingleApplication * pApp();
    static const QList<QAction *> childrenActions(QObject * main_object);
    static void makeCentered(QWidget * wnd);
    static const QString fixButtonText(const QString & label);
    template<typename T> static T iniValue(const QString & key) {
        QSettings settings;
        return settings.value("settings/"+key).value<T>();
    }
    template<typename T> static T iniValue(const QString & key,const T & defValue) {
        QSettings settings;
        return settings.value("settings/"+key,(QVariant)defValue).value<T>();
    }
    static void setIniValue(const QString & key,const QVariant & value);
    static void runDetachedUnderOrigUser(const QString & program,const QStringList & args);
    static const QSize quadroSize(int dimension);
    static void setupTranslations(const QString & mainName,const QDir & installDir,const QDir & mainLocalDir,const QDir & alpmLocalDir,const QDir & libLocalDir);

private:
    static void init_tr_variables();
};

#endif // STATIC_H
