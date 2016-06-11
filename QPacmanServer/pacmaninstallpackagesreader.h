/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanInstallPackagesReader_H
#define PacmanInstallPackagesReader_H

#include "pacmanprocessreader.h"
#include <QMap>

class PacmanInstallPackagesReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanInstallPackagesReader(const QString & packages,QObject *parent = 0);
    ~PacmanInstallPackagesReader();
    QStringList install_packages() const;
    QStringList remove_packages() const;
    void sendChosenProvider(const QString & provider);
    void sendAnswer(int answer);
    void beginInstall();
    void cancelInstall();
    double total_installed();
    double total_removed();

    static const QString providerChooserStr;

protected:
    virtual QString command() const;
    void error(const QString & error);

protected slots:
    void readyReadStandardError();
    void readyReadStandardOutput();
    virtual void onFinished(int code,QProcess::ExitStatus status);
    void start();

signals:
    void some_providers_available(const QStringList & providers);
    void question_available(const QString & question);
    void post_messages(const QString & package_name,const QStringList & messages);
    void ready_to_process(int cnt);
    void start_installing(const QString & name);
    void start_removing(const QString & name);
    void start_download(const QString & url);
    void all_downloads_completed();
    void download_progress(int percents);
    void contents_length_found(int length);

protected:
    QString in_packages;
    bool packagesRead;
    QStringList m_install_packages;
    QStringList m_remove_packages;
    QString tempConf;

private:
    bool packagesWasRead;
    QString current_installing;
    int countRead;
    QMap<QString,int> currentProviders;
    QMap<QString,QStringList> m_messages;
    QString m_outErrors;
    bool install_wait;
    int packages_count; // removal + installing ones
    double m_total_installed;
    double m_total_removed;
    bool packagesRetrieving;
    bool installCanceled;
    QString warnings;
};

#endif // PacmanInstallPackagesReader_H
