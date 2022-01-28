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

    setTimerInterval(timeout);
    setFinished(m_reply->isFinished());
    setOpenMode(ReadOnly);

    if (m_reply->error() != QNetworkReply::NoError) {
        setError(m_reply->error(),m_reply->errorString());
        QMetaObject::invokeMethod(this,"errorInternal",Qt::QueuedConnection,Q_ARG(QNetworkReply::NetworkError,error()));
        setFinished(true);
    }
    if (!isFinished()) {
        connect(m_reply,&QNetworkReply::metaDataChanged,this,[=]() { applyMetaData(); });
        connect(m_reply,&QNetworkReply::readyRead,this,[=]() { handleReadyRead(); });
        connect(m_reply,&QNetworkReply::errorOccurred,this,&NetworkReplyProxy::errorInternal);
        connect(m_reply,&QNetworkReply::finished,this,&NetworkReplyProxy::handleFinished);
        connect(m_reply,&QNetworkReply::uploadProgress,this,&NetworkReplyProxy::uploadProgress);
        connect(m_reply,&QNetworkReply::downloadProgress,this,&NetworkReplyProxy::downloadProgress);
        connect(m_reply,&QNetworkReply::sslErrors,this,&NetworkReplyProxy::sslErrors);
        connect(m_reply,&QNetworkReply::redirectAllowed,this,&NetworkReplyProxy::redirectAllowed);
        connect(m_reply,&QNetworkReply::aboutToClose,this,&NetworkReplyProxy::aboutToClose);
        connect(m_reply,&QNetworkReply::redirected,this,&NetworkReplyProxy::redirected);
        connect(m_reply,&QNetworkReply::preSharedKeyAuthenticationRequired,this,&NetworkReplyProxy::preSharedKeyAuthenticationRequired);
        connect(&m_timer,&QTimer::timeout,this,&NetworkReplyProxy::timeout);

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
    if (!isOpen()) return -1;
    if (m_buffer.size() <= 0) return 0;

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

    for(const QNetworkReply::RawHeaderPair & header: m_reply->rawHeaderPairs())
        setRawHeader(header.first, header.second);

    for (const QNetworkRequest::Attribute & attr: {QNetworkRequest::HttpStatusCodeAttribute,
                                                   QNetworkRequest::HttpReasonPhraseAttribute,
                                                   QNetworkRequest::RedirectionTargetAttribute,
                                                   QNetworkRequest::ConnectionEncryptedAttribute,
                                                   QNetworkRequest::SourceIsFromCacheAttribute,
                                                   QNetworkRequest::HttpPipeliningWasUsedAttribute,
                                                   QNetworkRequest::BackgroundRequestAttribute,
                                                   QNetworkRequest::SpdyWasUsedAttribute,
                                                   QNetworkRequest::Http2WasUsedAttribute,
                                                   QNetworkRequest::EmitAllUploadProgressSignalsAttribute,
                                                   QNetworkRequest::OriginalContentLengthAttribute})
        setAttribute(attr, m_reply->attribute(attr));

    if (signal)  {
       emit metaDataChanged();
       if (m_timer.interval() > 0) m_timer.start();
    }
}

void NetworkReplyProxy::errorInternal(QNetworkReply::NetworkError _error) {
    setError(_error,m_reply->errorString());
    emit errorOccurred(_error);
}

void NetworkReplyProxy::handleReadyRead(bool is_finished) {
    m_timer.stop();
    m_buffer += m_reply->readAll();
    if (is_finished) {
        if (m_buffer.size() > 0) emit readyRead();
        QMetaObject::invokeMethod(this,"handleFinished",Qt::QueuedConnection);
    }
    else {
        emit readyRead();
        if (m_timer.interval() > 0) m_timer.start();
    }
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
    emit errorOccurred(QNetworkReply::TimeoutError);
    abort();
}
