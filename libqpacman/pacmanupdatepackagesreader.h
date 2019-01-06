/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanUpdatePackagesReader_H
#define PacmanUpdatePackagesReader_H

#include "pacmanprocessreader.h"
#include <QMap>

class LIBQPACMANSHARED_EXPORT PacmanUpdatePackagesReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanUpdatePackagesReader(const QString & su_password,QObject *parent = 0);
    ~PacmanUpdatePackagesReader();
    QStringList install_packages() const;
    QStringList remove_packages() const;
    void sendChosenProvider(const QString & provider);
    void sendAnswer(char answer);
    void beginInstall();
    void cancelInstall();
    double total_installed();
    double total_removed();

    static const QString providerChooserStr;

protected:
    virtual QString command() const;
    bool error(const QString & errorOut);
    bool output(const QString & out);
    void waitForAllOutput();

protected slots:
    virtual void onFinished(int code,QProcess::ExitStatus status);
    void start();

signals:
    void some_providers_available(const QStringList & providers);
    void question_available(const QString & question);
    void post_messages(const QString & package_name,const QStringList & messages);
    void ready_to_process(double total_installed, double total_removed);
    void start_installing(const QString & name);
    void start_removing(const QString & name);
    void start_download(const QString & url);
    void all_downloads_completed();
    void download_progress(int percents);
    void contents_length_found(int length);

protected:
    bool packagesRead;
    QStringList m_install_packages;
    QStringList m_remove_packages;
    QString tempConf;

private:
    bool packagesWasRead;
    QString current_installing;
    QMap<QString,int> currentProviders;
    QMap<QString,QStringList> m_messages;
    QString m_outErrors;
    bool install_wait;
    double m_total_installed;
    double m_total_removed;
    bool packagesRetrieving;
    bool installCanceled;
    QString warnings;
    QString errorStream;
};

#endif // PacmanUpdatePackagesReader_H
