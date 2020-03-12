/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "networkreplyproxy.h"

NetworkReplyProxy::NetworkReplyProxy(QNetworkReply* reply, int timeout, QObject* parent)
  : QNetworkReply(parent), m_reply(reply) {

    m_reply->setParent(this);
    setOperation(m_reply->operation());
    setRequest(m_reply->request());
    setUrl(m_reply->url());
    setReadBufferSize(m_reply->readBufferSize());
    setSslConfiguration(m_reply->sslConfiguration());
    applyMetaData(false);

    m_timer.setInterval(0);
    setFinished(m_reply->isFinished());
    setOpenMode(ReadOnly);

    if (m_reply->error() != QNetworkReply::NoError) {
        setError(m_reply->error(),m_reply->errorString());
        QMetaObject::invokeMethod(this,"errorInternal",Qt::QueuedConnection,Q_ARG(QNetworkReply::NetworkError,error()));
        setFinished(true);
    }
    if (!isFinished()) {
        connect(m_reply,SIGNAL(metaDataChanged()),SLOT(applyMetaData()));
        connect(m_reply,SIGNAL(readyRead()),SLOT(handleReadyRead()));
        connect(m_reply,SIGNAL(error(QNetworkReply::NetworkError)),SLOT(errorInternal(QNetworkReply::NetworkError)));
        connect(m_reply,SIGNAL(finished()),SLOT(handleFinished()));
        connect(m_reply,SIGNAL(uploadProgress(qint64,qint64)),SIGNAL(uploadProgress(qint64,qint64)));
        connect(m_reply,SIGNAL(downloadProgress(qint64,qint64)),SIGNAL(downloadProgress(qint64,qint64)));
        connect(m_reply,SIGNAL(sslErrors(const QList<QSslError> &)),SIGNAL(sslErrors(const QList<QSslError> &)));
        connect(&m_timer,SIGNAL(timeout()),SLOT(timeout()));

        m_timer.setInterval(timeout);
        if (m_timer.interval() > 0) m_timer.start();
    }
    else QMetaObject::invokeMethod(this,"handleReadyRead",Qt::QueuedConnection,Q_ARG(bool,true));
}

void NetworkReplyProxy::abort() { m_reply->abort(); }
void NetworkReplyProxy::close() { m_reply->close(); }
bool NetworkReplyProxy::isSequential() const { return m_reply->isSequential(); }

void NetworkReplyProxy::handleFinished() {
    m_timer.stop();
    setFinished(true);
    emit finished();
}

qint64 NetworkReplyProxy::bytesAvailable() const {
    return m_buffer.size() + m_reply->bytesAvailable();
}

qint64 NetworkReplyProxy::readData(char* data, qint64 maxlen) {
    qint64 size = qMin(maxlen, qint64(m_buffer.size()));
    memcpy(data, m_buffer.constData(), size);
    m_buffer.remove(0, size);
    return size;
}

void NetworkReplyProxy::ignoreSslErrors() {
    m_reply->ignoreSslErrors();
}

void NetworkReplyProxy::applyMetaData(bool signal) {
    if (signal) m_timer.stop();

    foreach(QNetworkReply::RawHeaderPair header, m_reply->rawHeaderPairs())
        setRawHeader(header.first, header.second);

    for (QNetworkRequest::Attribute attr: {QNetworkRequest::HttpStatusCodeAttribute,
                                           QNetworkRequest::HttpReasonPhraseAttribute,
                                           QNetworkRequest::RedirectionTargetAttribute,
                                           QNetworkRequest::ConnectionEncryptedAttribute,
                                           QNetworkRequest::SourceIsFromCacheAttribute,
                                           QNetworkRequest::HttpPipeliningWasUsedAttribute,
                                           QNetworkRequest::BackgroundRequestAttribute,
                                           QNetworkRequest::SpdyWasUsedAttribute,
                                           QNetworkRequest::HTTP2WasUsedAttribute,
                                           QNetworkRequest::EmitAllUploadProgressSignalsAttribute,
                                           QNetworkRequest::OriginalContentLengthAttribute})
        setAttribute(attr, m_reply->attribute(attr));

    if (signal)  {
       if (m_timer.interval() > 0) m_timer.start();
       emit metaDataChanged();
    }
}

void NetworkReplyProxy::errorInternal(QNetworkReply::NetworkError _error) {
    setError(_error,m_reply->errorString());
    emit error(_error);
}

void NetworkReplyProxy::readInternal() {
    if (m_timer.interval() > 0) m_timer.start();
    m_buffer += m_reply->readAll();
    m_timer.stop();
}

void NetworkReplyProxy::handleReadyRead(bool is_finished) {
    m_timer.stop();
    readInternal();
    emit readyRead();
    if (is_finished) QMetaObject::invokeMethod(this,"handleFinished",Qt::QueuedConnection);
    else if (m_timer.interval() > 0) m_timer.start();
}

int NetworkReplyProxy::timerInterval() const {
    return m_timer.interval();
}

void NetworkReplyProxy::setTimerInterval(int value) {
    if (value <= 0) {
        value = 0;
        m_timer.stop();
    }
    m_timer.setInterval(value);
}

void NetworkReplyProxy::timeout() {
    m_timer.stop();
    setError(QNetworkReply::TimeoutError,tr("Timeout interval is reached because of long device inactivity!"));
    emit error(QNetworkReply::TimeoutError);
    abort();
}
