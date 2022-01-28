/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "alpmdownloader.h"
#include <QFileInfo>
#include <QFile>
#include "multidownloader.h"
#include "simpledownloader.h"
#include "libalpm.h"
#include <QSettings>
#include <QDebug>


AlpmDownloader::AlpmDownloader(const QUrl & url,const QDir & out_dir,int threads_count,int timeout,const QNetworkProxy & proxy,QObject *parent) : QEventLoop(parent) {
    m_out_dir = out_dir;
    init(url,threads_count,timeout,proxy);
}

AlpmDownloader::~AlpmDownloader() {
    delete downloader;
}

void AlpmDownloader::location_changed(const QUrl & url) {
    int threads_count = downloader->threadsCount();
    int timeout = downloader->timeout();
    QNetworkProxy proxy = downloader->proxy();
    downloader->deleteLater();
    init(url,threads_count,timeout,proxy);
}

void AlpmDownloader::init(const QUrl & url,int threads_count,int timeout,const QNetworkProxy & proxy) {
    m_downloaded = 0;
    downloader = new MultiDownloader(url,threads_count,m_out_dir.path());
    downloader->setTimeout(timeout);
    downloader->setProxy(proxy);
    init2();
}

void AlpmDownloader::init2() {
    m_downloaded = 0;

    connect(downloader,&DownloaderInterface::download_completed,this,&AlpmDownloader::download_completed);
    if (downloader->inherits("MultiDownloader"))
        connect((MultiDownloader *)downloader,&MultiDownloader::download_impossible,this,&AlpmDownloader::download_impossible);
    connect(downloader,&DownloaderInterface::progress,this,&AlpmDownloader::download_progress);
    connect(downloader,&DownloaderInterface::error_occured,this,&AlpmDownloader::download_error);
    connect(downloader,&DownloaderInterface::location_changed,this,&AlpmDownloader::location_changed);
    QMetaObject::invokeMethod(this,"start",Qt::QueuedConnection);
}

void AlpmDownloader::download_impossible() {
    QUrl url = downloader->url();
    int timeout = downloader->timeout();
    QNetworkProxy proxy = downloader->proxy();
    downloader->deleteLater();
    downloader = new SimpleDownloader(url,m_out_dir.path());
    downloader->setTimeout(timeout);
    downloader->setProxy(proxy);
    init2();
}

void AlpmDownloader::download_progress(qint64 downloaded,int percents,qint64 speed) {
    if (m_out_file_name.isEmpty()) m_out_file_name = QFileInfo(downloader->outputName()).fileName();

    m_downloaded = downloaded;

    if (ThreadRun::isTerminateFlagSet()) terminate();
    else emit progress(m_out_file_name,downloaded,downloader->dataLength(),percents,speed);
}

bool AlpmDownloader::removeFile(const QString & filename) {
    if (!QFileInfo::exists(filename)) return true;
    if (!QFile(filename).remove()) {
        emit error(tr("Cannot delete the following file: %1").arg(filename));
        if (isRunning()) exit(-1);
        return false;
    }
    return true;
}

void AlpmDownloader::start() {
    QString path = downloader->outputName();
    QString savePath = path+".save";
    QString renamedPath = path+".renamed";
    if (downloader->inherits("MultiDownloader") &&
            QFileInfo(savePath).isWritable() && QFileInfo(savePath).isReadable() &&
            QFileInfo(renamedPath).isWritable() && QFileInfo(renamedPath).isReadable()) {
        if (!removeFile(path)) return;
        renameFile(renamedPath,path);
        if (!((MultiDownloader *)downloader)->continueSaved(savePath)) {
            emit error(downloader->errorString());
            if (isRunning()) exit(-1);
        }
    }
    else {
        if (!removeFile(path)) return;
        if (!removeFile(savePath)) return;
        if (!removeFile(renamedPath)) return;
        if (!downloader->start()) {
            emit error(downloader->errorString());
            if (isRunning()) exit(-1);
        }
    }
}

QString AlpmDownloader::outputFileName() const {
    return m_out_file_name;
}

bool AlpmDownloader::renameFile(const QString & filename,const QString & new_filename) {
    QFile(filename).rename(new_filename);
    if (QFileInfo::exists(new_filename)) {
        QFile(filename).remove();
        return true;
    }
    return false;
}

void AlpmDownloader::save() {
    QString file_name = downloader->outputName();
    if (!downloader->inherits("MultiDownloader")) {
        QFile(file_name).remove();
        return;
    }
    QString savePath = file_name+".save";
    ((MultiDownloader *)downloader)->saveState(savePath);
    if (!renameFile(file_name,file_name+".renamed")) {
        QFile(file_name).remove();
        QFile(savePath).remove();
    }
}

void AlpmDownloader::download_error() {
    if (downloader->inherits("MultiDownloader")) save();
    emit error(downloader->errorString());
    qDebug() << downloader->errorString();
    if (isRunning()) exit(-1);
}

void AlpmDownloader::download_completed() {
    QFile(downloader->outputName()+".save").remove();
    emit progress(m_out_file_name,m_downloaded,(downloader->dataLength() > 0)?downloader->dataLength():m_downloaded,100,0);
    if (isRunning()) exit(0);
}

void AlpmDownloader::terminate() {
    downloader->terminate();
    if (downloader->inherits("MultiDownloader")) save();
    processEvents();
    if (isRunning()) exit(2);
}
