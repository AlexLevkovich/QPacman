/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef STATIC_H
#define STATIC_H

#include <QObject>
#include <QString>
#include <QTextDocumentFragment>
#include "libqpacman_global.h"
#include <QMainWindow>

class LIBQPACMANSHARED_EXPORT Static {
public:
    static QString Error_Str;
    static QString Warning_Str;
    static QString Question_Str;
    static QString Info_Str;
    static QString InstalledSuccess_Str;
    static QString PostMessages_Str;
    static QString RootRightsNeeded_Str;
    static QString PacmanTerminate_Str;
    static QString RepoAll_Str;
    static QString su_password;

    static bool checkRootAccess();
    static const QString arch();
    static QMainWindow * findMainWindow();
    static const QString userName();
    static const QString someProvidersAvailable(QWidget * parent,const QStringList & providers);
    static void init_tr_variables();
    static const QList<QAction *> childrenActions(QObject * main_object);
    static const QString htmlFragmentToText(const QTextDocumentFragment & fragment);
    static int isPackageInstalled(const QString & name);
    static void makeCentered(QWidget * wnd);
};

#endif // STATIC_H
