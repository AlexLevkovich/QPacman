/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef INSTALLPROGRESSLOOP_H
#define INSTALLPROGRESSLOOP_H

#include <QEventLoop>
#include "libqpacman_global.h"

class PacmanUpdatePackagesReader;
class PacmanProcessReader;

class LIBQPACMANSHARED_EXPORT InstallProgressLoop : public QEventLoop {
    Q_OBJECT
public:
    InstallProgressLoop(const QString & su_password,const QStringList & packages,QWidget *parent = 0);
    InstallProgressLoop(const QString & su_password,QWidget *parent = 0);

protected:
    explicit InstallProgressLoop(QWidget *parent = 0);
    virtual int packages_count();
    void init();

protected slots:
    void installing_packages_finished(PacmanProcessReader * reader);
    void start_download(const QString & url);
    void all_downloads_completed();
    void onCancel();
    void readyToProcess(double total_installed,double total_removed);
    void someProvidersAvailable(const QStringList & providers);
    void questionAvailable(const QString & question);

signals:
    void post_messages(const QString & package_name,const QStringList & messages);
    void start_waiting_mode();
    void end_waiting_mode();

protected:
    PacmanUpdatePackagesReader * installer;

private:
    bool wasCanceled;
    bool waiting_mode;
};

#endif // INSTALLPROGRESSDIALOG_H
