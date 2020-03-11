/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ALPMDOWNLOADER_H
#define ALPMDOWNLOADER_H

#include <QEventLoop>
#include <QUrl>
#include <QDir>
#include <QNetworkProxy>

class DownloaderInterface;

class AlpmDownloader : public QEventLoop {
    Q_OBJECT
public:
    AlpmDownloader(const QUrl & url,const QDir & out_dir,int threads_count = 8,int timeout = 0,const QNetworkProxy & proxy = QNetworkProxy(),QObject *parent = 0);
    ~AlpmDownloader();
    void terminate();
    QString outputFileName() const;

signals:
    void error(const QString & err);
    void progress(const QString & filename,qint64 bytes_downloaded,qint64 length,int percents,qint64 speed);

private slots:
    void location_changed(const QUrl & url);
    void download_completed();
    void download_impossible();
    void download_error();
    void download_progress(qint64 downloaded,int percents,qint64 speed);
    void start();

private:
    bool removeFile(const QString & filename);
    bool renameFile(const QString & filename,const QString & new_filename);
    void save();
    void init(const QUrl & url,int threads_count = 8,int timeout = 0,const QNetworkProxy & proxy = QNetworkProxy());
    void init2();

    DownloaderInterface * downloader;
    QDir m_out_dir;
    qint64 m_downloaded;
    QString m_out_file_name;
};

#endif // ALPMDOWNLOADER_H
