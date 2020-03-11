/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef SIMPLEDOWNLOADER_H
#define SIMPLEDOWNLOADER_H

#include "downloaderinterface.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QFile>

class QNetworkAccessManager;

class SimpleDownloader : public DownloaderInterface {
    Q_OBJECT
public:
    SimpleDownloader(const QUrl & url,const QString & outputName);

    bool isStarted();
    bool start();
    QString errorString();
    qint64 dataLength();
    bool terminate();
    QUrl url();
    bool setUrl(const QUrl & url);
    QString outputName();
    bool setOutputName(const QString & outputName);
    void setProxy(const QNetworkProxy & proxy);
    QNetworkProxy proxy() const;
    int threadsCount();
    int timeout() { return m_timeout; }
    void setTimeout(uint value) { m_timeout = (int)value; }

protected:
    void setErrorString(const QString & error);
    void setDataLength(qint64 size);

private slots:
    void private_start();
    void was_error(QNetworkReply::NetworkError);
    void metaDataChanged();
    void get_finished();
    void get_readyRead();
    void get_downloadProgress(qint64 downloaded,qint64 total);


private:
    void updateModiffTime();
    bool correctOutputFilePath(QNetworkReply * reply);
    QNetworkReply * get(const QNetworkRequest & request);
    void was_error(const QString & error,QNetworkReply * reply = NULL);

    QString m_outputName;
    qint64 m_size;
    QString m_error;
    QUrl m_url;
    bool m_is_started;
    QNetworkAccessManager * m_manager;
    qint64 m_downloaded;
    bool m_terminated;
    QFile m_out_file;
    qint64 m_start_msec;
    QDateTime m_modif_time;
    int m_timeout;
};

#endif // SIMPLEDOWNLOADER_H
