/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanInstallPackagesReader_H
#define PacmanInstallPackagesReader_H

#include "pacmanprocessreader.h"
#include <QStringList>

class PacmanInstallPackagesReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanInstallPackagesReader(const QString & packages,QObject *parent = 0);
    QStringList install_packages() const;
    QStringList remove_packages() const;
    void sendChosenProvider(const QString & provider);
    void beginInstall();
    void cancelInstall();
    void sendAnswer(char answer);

protected:
    virtual void send_parameters();
    virtual QByteArray command() const;

protected slots:
    virtual void on_readyToProcess(const QStringList & install_packages,const QStringList & remove_packages,const QStringList & local_install_packages,qreal total_installed, qreal total_removed);

private:
    QString m_packages;

signals:
    void some_providers_available(const QStringList & providers);
    void ready_to_process(qreal total_installed, qreal total_removed);
    void post_messages(const QString & package_name,const QStringList & messages);
    void start_installing(const QString & name);
    void start_removing(const QString & name);
    void contents_length_found(int len);
    void download_progress(int percents);
    void start_download(const QString & url);
    void all_downloads_completed();
    void question_available(const QString & question);

protected:
    QStringList m_install_packages;
    QStringList m_remove_packages;
};

#endif // PacmanInstallPackagesReader_H
