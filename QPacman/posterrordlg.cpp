/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "posterrordlg.h"
#include "static.h"
#include "errordialog.h"
#include <QApplication>

PostErrorDlg::PostErrorDlg(const QString & error,const QString & command) : QObject(NULL) {
    QMetaObject::invokeMethod(this,"_show_error",Qt::QueuedConnection,Q_ARG(const QString &,error),Q_ARG(const QString &,command));
}

void PostErrorDlg::_show_error(const QString & error,const QString & command) {
    QMainWindow * mainWindow = Static::findMainWindow();
    ErrorDialog(tr("Error(s) during executing of the command:\n%1").arg(command),error,mainWindow).exec();
    deleteLater();
    if (mainWindow->inherits("LocalPackageMainWindow")) {
        qApp->quit();
    }
}
