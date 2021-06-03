/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "multidownloader.h"
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QDataStream>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QNetworkAccessManager>
#include "networkreplyproxy.h"

#define DEF_PART_LENGTH 524288
#define MIN_RANGE_LENGTH 500
#define TIMER_INTERVAL 2000
#define CONNECT_TRY_COUNT 5

template<class ForwardIt, class T> static ForwardIt binary_search_ex(ForwardIt first, ForwardIt last, const T& value) {
    ForwardIt it = std::lower_bound(first, last, value);
    if ((it == last) || (value < *it)) it = last;
    return it;
}

PartManager::PartManager(const QString & outputName,qint64 part_length) {
    m_last_added_part_id = 0;
    m_progress = 0;
    m_part_length = (part_length <= 0)?DEF_PART_LENGTH:part_length;
    m_file.setFileName(outputName);
}

bool PartManager::open() {
    if (!m_file.open(QIODevice::WriteOnly)) {
        m_error = m_file.errorString();
        return false;
    }
    m_modif_time = QDateTime();
    return true;
}

bool PartManager::isOpen() {
    return m_file.isOpen();
}

void PartManager::close() {
    m_file.close();
    DownloaderInterface::setFileDate(m_file.fileName(),m_modif_time);
}

QString PartManager::outputName() const {
    return m_file.fileName();
}

void PartManager::setOutputName(const QString & outputName) {
    m_error.clear();
    m_parts.clear();
    m_progress = 0;
    m_last_added_part_id = 0;
    bool was_open = isOpen();
    if (was_open) close();
    m_file.setFileName(outputName);
    if (was_open) open();
}

bool PartManager::clear() {
    m_error.clear();
    m_parts.clear();
    m_progress = 0;
    m_last_added_part_id = 0;
    if (isOpen()) close();
    return open();
}

void PartManager::setPartLength(qint64 part_length) {
    if (part_length < 0) return;
    m_part_length = part_length;
}

int PartManager::createOrFindEmptyPart(qint64 maxSize,const QList<int> & ignore_indexes) {
    if (!isOpen()) return -1;

    Part part;
    if (m_parts.isEmpty()) {
        part.begin_pos = 0;
        part.curr_pos = 0;
        part.rest = m_part_length;
        if (m_part_length > maxSize) part.rest = maxSize;
        m_parts.append(part);
        m_last_added_part_id = 0;

        return part_zero_fill(0);
    }
    else {
        int i;
        for (i=(m_last_added_part_id + 1);i < m_parts.count();i++) {
            if (!partIsFull(i) && !ignore_indexes.contains(i)) {
                m_last_added_part_id = i;
                return i;
            }
        }

        if (!isLastPart(m_parts.count()-1,maxSize)) {
            part.begin_pos = part.curr_pos = m_parts.last().next_part_begin_pos();
            part.rest = m_part_length;
            if (part.next_part_begin_pos() > maxSize) part.rest -= part.next_part_begin_pos() - maxSize;
            else if ((maxSize - part.next_part_begin_pos()) <= MIN_RANGE_LENGTH) part.rest += (maxSize - part.next_part_begin_pos());
            m_parts.append(part);
            m_last_added_part_id = m_parts.count() - 1;
            return part_zero_fill(m_last_added_part_id);
        }

        for (i=0;i < m_last_added_part_id;i++) {
            if (!partIsFull(i) && !ignore_indexes.contains(i)) {
                m_last_added_part_id = i;
                return i;
            }
        }
    }

    return -1;
}

int PartManager::findPartByPos(qint64 pos) {
    Part search_part;
    search_part.begin_pos = 0;
    search_part.curr_pos = pos;
    search_part.rest = -1;
    QList<Part>::iterator i = binary_search_ex(m_parts.begin(), m_parts.end(), search_part);
    if (i == m_parts.end()) return -1;
    return i - m_parts.begin();
}

bool PartManager::reservePartsForPos(qint64 pos,qint64 maxSize) {
    if (pos < m_parts.last().next_part_begin_pos()) {
        int part_id = findPartByPos(pos);
        if (part_id < 0) return false;
        m_last_added_part_id = (part_id == 0)?part_id:part_id - 1;
        return true;
    }

    for (int i=m_parts.count()-1;!hasLastPart(maxSize);i++) {
        m_last_added_part_id = i;
        if (createOrFindEmptyPart(maxSize) < 0) return false;
        if (m_parts.last().contains(pos)) {
            int part_id = m_parts.count() - 1;
            m_last_added_part_id = (part_id == 0)?part_id:part_id - 1;
            return true;
        }
    }

    return false;
}

int PartManager::part_zero_fill(int id) {
    if (!isOpen()) return -1;

    Part part = m_parts.at(id);
    int part_rest = part.rest;
    m_file.seek(part.curr_pos);

    int ret;
    while (part_rest > 0) {
        ret = m_file.write(QByteArray(part_rest,(char)0));
        if (ret < 0) {
            m_error = m_file.errorString();
            return -1;
        }
        part_rest -= ret;
    }

    return id;
}

int PartManager::writeToPart(int part_id,char * data,int len) {
    if (!isOpen()) return 0;

    if (len <= 0 || data == NULL || !isValid()) {
        m_error = QObject::tr("Nothing to write!!!");
        return 0;
    }

    Part & part = m_parts[part_id];

    if (part.rest <= 0) {
        m_error = QObject::tr("No way to write. Part is full!!!");
        return 0;
    }

    m_file.seek(part.curr_pos);

    int ret;
    int written = 0;
    bool warning = false;
    if (len > part.rest) {
        warning = true;
        len = part.rest;
    }

    while (written < len) {
        ret = m_file.write(data+written,len-written);
        if (ret < 0) {
            m_error = m_file.errorString();
            return 0;
        }
        part.rest -= ret;
        part.curr_pos += ret;
        written += ret;
        m_progress += ret;
    }

    return warning?2:1;
}

int PartManager::writeToPart(int part_id,const QByteArray & data) {
    if (!isOpen()) return 0;

    if (data.length() <= 0 || !isValid()) {
        m_error = QObject::tr("Nothing to write!!!");
        return 0;
    }

    Part & part = m_parts[part_id];

    if (part.rest <= 0) {
        m_error = QObject::tr("No way to write. Part is full!!!");
        return 0;
    }

    m_file.seek(part.curr_pos);

    int ret;
    int written = 0;
    int len = data.length();
    bool warning = false;
    if (len > part.rest) {
        warning = true;
        len = part.rest;
    }

    while (written < len) {
        ret = m_file.write(data.data()+written,len-written);
        if (ret < 0) {
            m_error = m_file.errorString();
            return 0;
        }
        part.rest -= ret;
        part.curr_pos += ret;
        written += ret;
        m_progress += ret;
    }

    return warning?2:1;
}

QList<int> PartManager::incompletedPartIndexes() {
    QList<int> ret_list;
    for (int i=0;i<m_parts.count();i++) {
        if (!partIsFull(i)) ret_list.append(i);
    }
    std::sort(ret_list.begin(),ret_list.end());
    return ret_list;
}

QDataStream & operator<<(QDataStream &stream, const PartManager::Part & part) {
    stream << part.begin_pos;
    stream << part.curr_pos;
    stream << part.rest;
    return stream;
}

QDataStream & operator>>(QDataStream &stream,PartManager::Part & part) {
    stream >> part.begin_pos;
    stream >> part.curr_pos;
    stream >> part.rest;
    return stream;
}

QDataStream & operator<<(QDataStream &stream, const PartManager & part_manager) {
    stream << part_manager.m_part_length;
    stream << part_manager.m_file.fileName();
    stream << part_manager.m_error;
    stream << part_manager.m_parts;
    stream << part_manager.m_progress;
    stream << part_manager.m_last_added_part_id;
    return stream;
}

QDataStream & operator>>(QDataStream &stream,PartManager & part_manager) {
    part_manager.m_error.clear();
    stream >> part_manager.m_part_length;
    QString file_name;
    stream >> file_name;
    if (part_manager.m_file.isOpen()) part_manager.m_file.close();
    part_manager.m_file.setFileName(file_name);
    if (!part_manager.m_file.open(QIODevice::ReadWrite)) part_manager.m_error = part_manager.m_file.errorString();
    QString m_error;
    stream >> m_error;
    if (part_manager.m_error.isEmpty()) part_manager.m_error = m_error;
    stream >> part_manager.m_parts;
    stream >> part_manager.m_progress;
    stream >> part_manager.m_last_added_part_id;
    return stream;
}


MultiDownloader::MultiDownloader(const QUrl & url,int threads_count,const QString & outputName) : DownloaderInterface() {
    m_url = url;
    m_threads_count = threads_count;
    m_connect_attempt_count = CONNECT_TRY_COUNT;
    m_size = 0;
    m_prev_bytes_written = 0;
    m_timeout = 0;

    m_manager = new QNetworkAccessManager(this);
    QString m_outputName = outputName;
    if (!outputName.isEmpty() && !url.fileName().isEmpty() && QFileInfo(outputName).isDir()) {
        m_outputName = QDir(m_outputName).path()+QDir::separator()+url.fileName();
    }
    m_part_manager = new PartManager(m_outputName,0);
    m_timer = new QTimer(this);

    connect(m_timer,SIGNAL(timeout()),this,SLOT(ontimeout()));
}

void MultiDownloader::setProxy(const QNetworkProxy & proxy) {
    m_manager->setProxy(proxy);
}

QNetworkProxy MultiDownloader::proxy() const {
    return m_manager->proxy();
}

MultiDownloader::~MultiDownloader() {
    delete m_part_manager;
}

QUrl MultiDownloader::url() {
    return m_url;
}

bool MultiDownloader::setUrl(const QUrl & url) {
    if (isStarted()) return false;

    m_url = url;
    return true;
}

int MultiDownloader::reconnectAttemptCount() {
    return m_connect_attempt_count;
}

void MultiDownloader::setReconnectAttemptCount(int count) {
    m_connect_attempt_count = count;
}

void MultiDownloader::setThreadCount(int threads_count) {
    m_threads_count = threads_count;
}

int MultiDownloader::partLenght() {
    return m_part_manager->partLenght();
}

void MultiDownloader::setPartLength(int part_length) {
    if (part_length < 0) return;
    m_part_manager->setPartLength(part_length);
}

int MultiDownloader::threadsCount() {
    return m_threads_count;
}

bool MultiDownloader::start() {
    setErrorString("");

    if (isStarted()) {
        setErrorString(tr("Downloading already has been started!!!"));
        return false;
    }

    if (threadsCount() <= 0) {
        setErrorString(tr("Invalid count of threads has passed!!!"));
        return false;
    }

    if (m_manager == NULL) {
        setErrorString(tr("Invalid QNetworkAccessManager instance!!!"));
        return false;
    }

    if (!m_url.isValid()) {
        setErrorString(tr("The url is invalid!!!"));
        return false;
    }

    m_size = 0;
    m_prev_bytes_written = 0;

    invokeMethod("private_start");

    return true;
}

QNetworkReply * MultiDownloader::get(const QNetworkRequest & request) {
    QNetworkReply * m_reply = m_manager->get(request);
    if (m_reply == NULL) {
        was_error(tr("Returned wrong QNetworkReply pointer!!!"));
        return NULL;
    }
    m_reply = new NetworkReplyProxy(m_reply,m_timeout,this);
    return m_reply;
}

void MultiDownloader::private_start() {
    QNetworkReply * m_reply = get(QNetworkRequest(m_url));
    if (m_reply == NULL) return;

    m_reply->setProperty("type","main");
    connect(m_reply,SIGNAL(metaDataChanged()),this,SLOT(mainMetaDataChanged()));
    connect(m_reply,SIGNAL(errorOccurred(QNetworkReply::NetworkError)),this,SLOT(was_error(QNetworkReply::NetworkError)));
    connect(m_reply,&QNetworkReply::sslErrors,[=]() { m_reply->ignoreSslErrors(); });
}

void MultiDownloader::setDataLength(qint64 size) {
    m_size = size;
}

bool MultiDownloader::correctOutputFilePath(QNetworkReply * reply) {
    QString out_path = m_part_manager->outputName();
    if (out_path.isEmpty()) {
        was_error(tr("The path to output file was not passed!!!"),reply);
        return false;
    }
    if (QFileInfo(out_path).isDir()) {
        QString disposition = reply->header(QNetworkRequest::ContentDispositionHeader).toString();
        if (disposition.isEmpty() || !disposition.contains("filename=\"")) {
            was_error(tr("Cannot determine the output file name!!!"),reply);
            return false;
        }
        else {
            int index = disposition.indexOf("filename=\"")+10;
            m_part_manager->setOutputName(QDir(out_path).path()+QDir::separator()+disposition.mid(index,disposition.length()-index-1));
        }
    }
    return true;
}

void MultiDownloader::mainMetaDataChanged() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302 || statusCode == 308) {
        QByteArray new_url_data = m_reply->rawHeader("Location");
        if (!new_url_data.isEmpty()) {
            QUrl new_url = QUrl::fromEncoded(new_url_data);
            if (new_url != m_url) {
                was_error("",m_reply);
                emit location_changed(new_url);
                return;
            }
        }
    }

    if (statusCode >= 400) {
        was_error(tr("The server returned %1 code!").arg(statusCode),m_reply);
        return;
    }

    qint64 length = m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    if (length <= 0 || m_url.scheme().toLower() == "ftp") {
        was_error("",m_reply);
        emit download_impossible();
        return;
    }

    setDataLength(length);
    emit data_length_is_known(length);
    if (m_threads_count == 1 || m_part_manager->partLenght() > length) {
        setThreadCount(1);
        m_part_manager->setPartLength(length);
    }
    else {
        m_threads_count = qMin((qint64)m_threads_count,length/m_part_manager->partLenght());
        m_part_manager->setPartLength(length/m_threads_count);
    }
    if (!correctOutputFilePath(m_reply)) return;

    if (!m_part_manager->clear()) {
        was_error(m_part_manager->errorString(),m_reply);
        return;
    }

    QDateTime date = m_reply->header(QNetworkRequest::LastModifiedHeader).toDateTime();
    if (!date.isNull()) m_part_manager->m_modif_time = date;

    emit progress(0,0,0);
    m_reply->abort();
    m_reply->deleteLater();

    invokeMethod("addNewPartDownload");
    m_timer->start(TIMER_INTERVAL);
}

bool MultiDownloader::addNewPartDownload(int part_id,int try_counter) {
    QList<int> working_indexes = workingPartIndexes();

    if (m_part_manager->maxAllocatedLength() >= dataLength() && working_indexes == m_part_manager->incompletedPartIndexes()) return true;
    if (part_id < 0) {
        if (countWorkingParts() >= m_threads_count) return false;
        part_id = m_part_manager->createOrFindEmptyPart(dataLength(),working_indexes);
        if (part_id < 0) {
            was_error(m_part_manager->errorString());
            return false;
        }
    }
    else {
        if (!m_part_manager->partExists(part_id)) return false;
        if (m_part_manager->partIsFull(part_id)) return true;
    }

    QNetworkRequest part_request(m_url);
    qint64 curr_pos = m_part_manager->partCurrPos(part_id);
    if (m_threads_count > 1) part_request.setRawHeader("Range",QString("bytes=%1-%2").arg(curr_pos).arg(curr_pos+m_part_manager->partRest(part_id)-1).toLatin1());
    QNetworkReply * m_reply = get(part_request);
    if (m_reply == NULL) return false;

    m_reply->setProperty("type","child");
    m_reply->setProperty("part",part_id);
    m_reply->setProperty("try_counter",try_counter);
    connect(m_reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(was_error(QNetworkReply::NetworkError)));
    connect(m_reply,SIGNAL(finished()),this,SLOT(child_finished()));
    connect(m_reply,SIGNAL(readyRead()),this,SLOT(child_readyRead()));
    connect(m_reply,SIGNAL(metaDataChanged()),this,SLOT(childMetaDataChanged()));
    connect(m_reply,&QNetworkReply::sslErrors,[=]() { m_reply->ignoreSslErrors(); });

    return true;
}

void MultiDownloader::childMetaDataChanged() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

    if (m_threads_count > 1) {
        int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode != 206) {
            was_error("",m_reply);
            emit download_impossible();
            return;
        }
    }

    invokeMethod("addNewPartDownload");
}

void MultiDownloader::was_error(QNetworkReply::NetworkError error) {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

    if (error == QNetworkReply::OperationCanceledError) return;

    was_error(m_reply->errorString(),m_reply);
}

void MultiDownloader::was_error(const QString & error,QNetworkReply * reply) {
    if (reply != NULL && error.isEmpty()) {
        m_timer->stop();
        m_part_manager->close();
        reply->abort();
        reply->deleteLater();
    }
    else if (reply == NULL && !error.isEmpty()) {
        setErrorString(error);
        m_timer->stop();
        m_part_manager->close();
        emit error_occured();
    }
    else if (reply != NULL && reply->property("type").toString() == "main") {
        setErrorString(error);
        m_timer->stop();
        m_part_manager->close();

        reply->abort();
        reply->deleteLater();
        emit error_occured();
    }
    else if (reply != NULL) {
        int try_count = m_connect_attempt_count;
        if (reply != NULL) try_count = reply->property("try_counter").toInt();
        if (reply != NULL && try_count < m_connect_attempt_count) {
            try_count++;
            reply->abort();
            reply->deleteLater();
            invokeMethod("addNewPartDownload",Q_ARG(int,reply->property("part").toInt()),Q_ARG(int,try_count));
        }
        else {
            setErrorString(error);
            m_timer->stop();
            m_part_manager->close();
            reply->abort();
            reply->deleteLater();
            emit error_occured();
        }
    }
}

void MultiDownloader::child_readyRead() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

    m_reply->setProperty("try_counter",1);
    int part_id = m_reply->property("part").toInt();
    if (m_part_manager->partIsFull(part_id)) {
        m_reply->readAll();
        return;
    }

    if (!m_part_manager->writeToPart(part_id,m_reply->readAll())) {
        was_error(m_part_manager->errorString(),m_reply);
        return;
    }
}

void MultiDownloader::child_finished() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();
    if (m_reply == NULL) return;
    if (!m_error.isEmpty()) return;

    m_reply->abort();
    m_reply->deleteLater();

    if (!isStarted()) {
        m_timer->stop();
        m_part_manager->close();
        emit download_terminated();
        return;
    }

    if (m_part_manager->writtenLength() >= dataLength()) {
        m_timer->stop();
        m_part_manager->close();
        emit progress(dataLength(),100,0);
        emit download_completed();
        return;
    }

    if (countWorkingParts() >= m_threads_count) return;

    invokeMethod("addNewPartDownload");
}

void MultiDownloader::ontimeout() {
    qint64 all_written = m_part_manager->writtenLength();

    if (all_written == m_prev_bytes_written) return;

    emit progress(all_written,(int)(((double)all_written)/((double)dataLength()/100.0)),(all_written - m_prev_bytes_written)/(TIMER_INTERVAL/1000));
    m_prev_bytes_written = all_written;
}

qint64 MultiDownloader::dataLength() {
    return m_size;
}

void MultiDownloader::private_terminate() {
    m_save_size = dataLength();
    setDataLength(0);
    QList<QNetworkReply *> list = findChildren<QNetworkReply *>();
    for (QNetworkReply * reply: list) {
        reply->abort();
    }
    m_timer->stop();
}

bool MultiDownloader::terminate() {
    if (!isStarted()) return true;

    invokeMethod("private_terminate");

    QEventLoop loop;
    connect(this,SIGNAL(download_completed()),&loop,SLOT(quit()));
    connect(this,SIGNAL(download_terminated()),&loop,SLOT(quit()));
    connect(this,SIGNAL(error_occured()),&loop,SLOT(quit()));
    loop.exec();

    setDataLength(m_save_size);

    return true;
}

bool MultiDownloader::saveState(const QString & outputName) {
    if (isStarted()) return false;

    QFile file(outputName);
    if (!file.open(QIODevice::WriteOnly)) return false;
    QDataStream stream(&file);
    stream << url();
    stream << threadsCount();
    stream << *m_part_manager;
    stream << errorString();
    stream << dataLength();
    stream << m_prev_bytes_written;
    stream << m_timeout;
    return (stream.status() == QDataStream::Ok);
}

void MultiDownloader::setErrorString(const QString & error) {
    m_error = error;
}

QString MultiDownloader::errorString() {
    return m_error;
}

bool MultiDownloader::continueSaved(const QString & inputName) {
    if (isStarted()) {
        setErrorString(tr("Downloading already has been started!!!"));
        return false;
    }

    QFile file(inputName);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QDataStream stream(&file);
    stream >> m_url;
    stream >> m_threads_count;
    stream >> *m_part_manager;
    stream >> m_error;
    stream >> m_size;
    stream >> m_prev_bytes_written;
    stream >> m_timeout;
    if (stream.status() != QDataStream::Ok) return false;

    if (m_size == 0) {
        file.close();
        file.remove();

        m_size = 0;
        m_prev_bytes_written = 0;

        invokeMethod("private_start");
    }
    else invokeMethod("private_continueSaved");

    return true;
}

void MultiDownloader::private_continueSaved() {
    bool started = false;

    qint64 all_written = m_part_manager->writtenLength();
    emit progress(all_written,(int)(((double)all_written)/((double)dataLength()/100.0)),(all_written - m_prev_bytes_written)/(TIMER_INTERVAL/1000));

    m_timer->start(TIMER_INTERVAL);
    QList<int> indexes = m_part_manager->incompletedPartIndexes();
    for (int i=0;i<qMin(m_threads_count,indexes.count());i++) {
        started = true;
        invokeMethod("addNewPartDownload",Q_ARG(int,indexes.at(i)));
    }

    if (!started) {
        if (countWorkingParts() >= m_threads_count) return;
        invokeMethod("addNewPartDownload");
    }
}

bool MultiDownloader::isStarted() {
    return m_timer->isActive();
}

QList<int> MultiDownloader::workingPartIndexes() {
    QList<int> ret_list;
    QList<QNetworkReply *> list = findChildren<QNetworkReply *>();
    for (int i=0;i<list.count();i++) {
        if (list.at(i)->property("type") != "child") continue;
        int part_id = list.at(i)->property("part").toInt();
        if (!m_part_manager->partIsFull(part_id)) ret_list.append(part_id);
    }
    std::sort(ret_list.begin(),ret_list.end());
    return ret_list;
}

int MultiDownloader::countWorkingParts() {
    QList<QNetworkReply *> childs = findChildren<QNetworkReply *>();
    int count = childs.count();
    for (int i=0;i<childs.count();i++) {
        if (childs[i]->property("type") != "child") count--;
    }
    return count;
}

QString MultiDownloader::outputName() {
    return m_part_manager->outputName();
}

bool MultiDownloader::setOutputName(const QString & outputName) {
    m_part_manager->setOutputName(outputName);
    return true;
}
