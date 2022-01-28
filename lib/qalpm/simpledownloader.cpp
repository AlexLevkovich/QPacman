/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "simpledownloader.h"
#include <QNetworkAccessManager>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include "networkreplyproxy.h"

SimpleDownloader::SimpleDownloader(const QUrl & url,const QString & outputName) : DownloaderInterface() {
    m_url = url;
    m_size = 0;
    m_is_started = false;
    m_downloaded = 0;
    m_terminated = false;
    m_timeout = 0;

    m_manager = new QNetworkAccessManager(this);
    m_outputName = outputName;
    if (!outputName.isEmpty() && !url.fileName().isEmpty() && QFileInfo(outputName).isDir()) {
        m_outputName = QDir(m_outputName).path()+QDir::separator()+url.fileName();
    }
}

void SimpleDownloader::setErrorString(const QString & error) {
    m_error = error;
}

QString SimpleDownloader::errorString() {
    return m_error;
}

bool SimpleDownloader::isStarted() {
    return m_is_started;
}

qint64 SimpleDownloader::dataLength() {
    return m_size;
}

bool SimpleDownloader::terminate() {
    if (!isStarted()) return false;

    m_terminated = true;
    QList<QNetworkReply *> list = findChildren<QNetworkReply *>();
    for (int i=0;i<list.count();i++) {
        list[i]->abort();
    }
    return true;
}

void SimpleDownloader::updateModiffTime() {
    setFileDate(m_out_file.fileName(),m_modif_time);
}

int SimpleDownloader::threadsCount() {
    return 1;
}

QUrl SimpleDownloader::url() {
    return m_url;
}

bool SimpleDownloader::setUrl(const QUrl & url) {
    if (isStarted()) return false;

    m_url = url;

    return true;
}

QString SimpleDownloader::outputName() {
    return m_outputName;
}

bool SimpleDownloader::setOutputName(const QString & outputName) {
    if (isStarted()) return false;

    m_outputName = outputName;

    return true;
}

void SimpleDownloader::setProxy(const QNetworkProxy & proxy) {
    m_manager->setProxy(proxy);
}

QNetworkProxy SimpleDownloader::proxy() const {
    return m_manager->proxy();
}

void SimpleDownloader::setDataLength(qint64 size) {
    m_size = size;
}

bool SimpleDownloader::start() {
    setErrorString("");

    if (isStarted()) {
        setErrorString(tr("Downloading already has been started!!!"));
        return false;
    }

    if (m_manager == NULL) {
        setErrorString(tr("Invalid QNetworkAccessManager instance!!!"));
        return false;
    }

    m_size = 0;
    m_downloaded = 0;
    m_terminated = false;
    m_is_started = true;

    invokeMethod("private_start");

    return true;
}

QNetworkReply * SimpleDownloader::get(const QNetworkRequest & request) {
    QNetworkReply * m_reply = m_manager->get(request);
    if (m_reply == NULL) {
        was_error_part(tr("Returned wrong QNetworkReply pointer!!!"));
        return NULL;
    }
    m_reply = new NetworkReplyProxy(m_reply,m_timeout,this);
    return m_reply;
}

void SimpleDownloader::private_start() {
    QNetworkReply * m_reply = get(QNetworkRequest(m_url));
    if (m_reply == NULL) return;

    connect(m_reply,&QNetworkReply::metaDataChanged,this,&SimpleDownloader::metaDataChanged);
    connect(m_reply,&QNetworkReply::errorOccurred,this,&SimpleDownloader::was_error);
    connect(m_reply,&QNetworkReply::finished,this,&SimpleDownloader::get_finished);
    connect(m_reply,&QNetworkReply::readyRead,this,&SimpleDownloader::get_readyRead);
    connect(m_reply,&QNetworkReply::downloadProgress,this,&SimpleDownloader::get_downloadProgress);
    connect(m_reply,&QNetworkReply::sslErrors,[=]() { m_reply->ignoreSslErrors(); });
}

void SimpleDownloader::was_error(QNetworkReply::NetworkError error) {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

    if (error == QNetworkReply::OperationCanceledError) return;

    was_error_part(m_reply->errorString(),m_reply);
}

void SimpleDownloader::was_error_part(const QString & error,QNetworkReply * reply) {
    m_out_file.close();
    updateModiffTime();
    m_is_started = false;
    if (reply != NULL) {
        reply->abort();
        reply->deleteLater();
    }
    if (!error.isEmpty()) {
        setErrorString(error);
        emit error_occured();
    }
}

bool SimpleDownloader::correctOutputFilePath(QNetworkReply * reply) {
    QString out_path = m_outputName;
    if (out_path.isEmpty()) {
        was_error_part(tr("The path to output file was not passed!!!"),reply);
        return false;
    }
    if (QFileInfo(out_path).isDir()) {
        QString disposition = reply->header(QNetworkRequest::ContentDispositionHeader).toString();
        if (disposition.isEmpty() || !disposition.contains("filename=\"")) {
            was_error_part(tr("Cannot determine the output file name!!!"),reply);
            return false;
        }
        else {
            int index = disposition.indexOf("filename=\"")+10;
            m_outputName = QDir(out_path).path()+QDir::separator()+disposition.mid(index,disposition.length()-index-1);
        }
    }
    return true;
}

void SimpleDownloader::metaDataChanged() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302 || statusCode == 308) {
        QByteArray new_url_data = m_reply->rawHeader("Location");
        if (!new_url_data.isEmpty()) {
            QUrl new_url = QUrl::fromEncoded(new_url_data);
            if (new_url != m_url) {
                was_error_part("",m_reply);
                emit location_changed(new_url);
                return;
            }
        }
    }

    if (statusCode >= 400) {
        was_error_part(tr("The server returned %1 code!").arg(statusCode),m_reply);
        return;
    }

    m_size = (qint64)m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    emit data_length_is_known(m_size);

    if (!correctOutputFilePath(m_reply)) return;

    m_out_file.setFileName(m_outputName);
    if (!m_out_file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        was_error_part(m_out_file.errorString(),m_reply);
        return;
    }

    m_modif_time = QDateTime();
    QDateTime date = m_reply->header(QNetworkRequest::LastModifiedHeader).toDateTime();
    if (!date.isNull()) m_modif_time = date;

    emit progress(0,0,0);
    m_start_msec = QDateTime::currentMSecsSinceEpoch();
}

void SimpleDownloader::get_finished() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();
    if (m_reply == NULL) return;
    if (!m_error.isEmpty()) return;

    m_reply->abort();
    m_reply->deleteLater();
    m_out_file.close();
    updateModiffTime();

    emit progress(m_downloaded,100,0);
    m_is_started = false;

    if (m_terminated) emit download_terminated();
    else emit download_completed();
}

void SimpleDownloader::get_readyRead() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();
    QByteArray data = m_reply->readAll();

    int ret;
    int written = 0;
    int len = data.length();
    while (written < len) {
        ret = m_out_file.write(data.data()+written,len-written);
        if (ret < 0) {
            was_error_part(m_out_file.errorString(),m_reply);
            return;
        }
        written += ret;
    }
    m_downloaded += len;
}

void SimpleDownloader::get_downloadProgress(qint64 downloaded,qint64) {
    emit progress(downloaded,m_size,(qint64)(((qreal)downloaded/(qreal)(QDateTime::currentMSecsSinceEpoch()-m_start_msec))*1000.0));
}
