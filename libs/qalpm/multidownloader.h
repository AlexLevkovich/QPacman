/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef MULTIDOWNLOADER_H
#define MULTIDOWNLOADER_H

#include <QThread>
#include <QUrl>
#include <QFile>
#include <QList>
#include <QMap>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QDataStream>
#include <QDateTime>
#include "downloaderinterface.h"

class QNetworkAccessManager;

class PartManager {
protected:
    PartManager(const QString & outputName,qint64 part_length);
    bool open();
    bool clear();
    void close();
    bool isOpen();
    inline void clearErrorString() { m_error.clear(); }
    inline qint64 partLenght() { return m_part_length; }
    void setPartLength(qint64 part_length);
    inline QString errorString() { return m_error; }
    inline bool isValid() { return m_file.isOpen() && m_error.isEmpty(); }
    // rc==-1 was error; rc>0 is part_id
    int createOrFindEmptyPart(qint64 maxSize,const QList<int> & ignore_indexes = QList<int>());
    bool reservePartsForPos(qint64 pos,qint64 maxSize);
    inline qint64 partCurrPos(int part_id) { return m_parts.at(part_id).curr_pos; }
    inline qint64 partRest(int part_id) { return m_parts.at(part_id).rest; }
    inline bool hasLastPart(qint64 maxSize) { return (m_parts.last().next_part_begin_pos() >= maxSize); }
    inline bool isLastPart(int part_id,qint64 maxSize) { return (m_parts.at(part_id).next_part_begin_pos() >= maxSize); }
    int findPartByPos(qint64 pos);
    // rc==0 was error; rc==1 is ok; rc==2 ok, with warning
    int writeToPart(int part_id,const QByteArray & data);
    int writeToPart(int part_id,char * data,int len);
    inline bool partIsFull(int part_id) { return (m_parts.at(part_id).rest <= 0); }
    inline bool partExists(int part_id) { return (part_id < m_parts.count()); }
    inline qint64 maxAllocatedLength() { return (m_parts.count() <= 0)?0:m_parts.last().next_part_begin_pos(); }
    inline qint64 writtenLength() { return m_progress; }
    QString outputName() const;
    void setOutputName(const QString & outputName);
    QList<int> incompletedPartIndexes();

private:
    int part_zero_fill(int id);

    struct Part {
        qint64 begin_pos;
        qint64 curr_pos;
        qint64 rest;

        inline qint64 next_part_begin_pos() const {
            return curr_pos + rest;
        }

        inline bool contains(qint64 pos) const {
            return (begin_pos <= pos) && (pos < next_part_begin_pos());
        }

        inline bool operator<(const Part & other) const {
            if (rest < 0) return other.contains(curr_pos);
            if (other.rest < 0) return (next_part_begin_pos() <= other.curr_pos);
            return curr_pos < other.curr_pos;
        }
        friend QDataStream & operator<<(QDataStream &stream, const Part & part);
        friend QDataStream & operator>>(QDataStream &stream, Part & part);
    };

    qint64 m_part_length;
    QFile m_file;
    QString m_error;
    QList<Part> m_parts;
    qint64 m_progress;
    int m_last_added_part_id;
    QDateTime m_modif_time;

    friend QDataStream & operator<<(QDataStream &stream, const Part & part);
    friend QDataStream & operator>>(QDataStream &stream, Part & part);
    friend QDataStream & operator<<(QDataStream &stream, const PartManager & part_manager);
    friend QDataStream & operator>>(QDataStream &stream, PartManager & part_manager);
    friend class MultiDownloader;
};

QDataStream & operator<<(QDataStream &stream, const PartManager::Part & part);
QDataStream & operator>>(QDataStream &stream, PartManager::Part & part);
QDataStream & operator<<(QDataStream &stream, const PartManager & part_manager);
QDataStream & operator>>(QDataStream &stream, PartManager & part_manager);

class MultiDownloader : public DownloaderInterface {
    Q_OBJECT
public:
    MultiDownloader(const QUrl & url = QUrl(),int threads_count = 1,const QString & outputName = QString());
    ~MultiDownloader();
    int partLenght();
    bool isStarted();
    void setPartLength(int part_length);
    bool start();
    bool continueSaved(const QString & inputName);
    QString errorString();
    qint64 dataLength();
    bool terminate();
    QUrl url();
    bool setUrl(const QUrl & url);
    int threadsCount();
    void setThreadCount(int threads_count);
    int reconnectAttemptCount();
    void setReconnectAttemptCount(int count);
    QString outputName();
    bool setOutputName(const QString & outputName);
    bool saveState(const QString & outputName);
    void setProxy(const QNetworkProxy & proxy);
    QNetworkProxy proxy() const;
    int timeout() { return m_timeout; }
    void setTimeout(uint value) { m_timeout = (int)value; }

private slots:
    void private_start();
    void private_terminate();
    void private_continueSaved();
    void was_error(QNetworkReply::NetworkError error);
    void child_finished();
    void mainMetaDataChanged();
    void childMetaDataChanged();
    void child_readyRead();
    void emit_download_completed();
    void emit_download_terminated();
    void ontimeout();
    bool addNewPartDownload(int part_id = -1,int try_counter = 1);

signals:
    void download_impossible(); //lenght is unknown or partial downloading is not supported by the server

protected:
    void setErrorString(const QString & error);
    void setDataLength(qint64 size);

private:
    bool correctOutputFilePath(QNetworkReply * reply);
    QNetworkReply * get(const QNetworkRequest & request);
    void was_error(const QString & error,QNetworkReply * reply = NULL);
    QList<int> workingPartIndexes();
    int countWorkingParts();

    QTimer * m_timer;
    QUrl m_url;
    int m_threads_count;
    int m_connect_attempt_count;
    PartManager * m_part_manager;
    QNetworkAccessManager * m_manager;
    QString m_error;
    qint64 m_size;
    qint64 m_prev_bytes_written;
    qint64 m_save_size;
    int m_timeout;
};

#endif // MULTIDOWNLOADER_H
