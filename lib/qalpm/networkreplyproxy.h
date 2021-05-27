/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef _NETWORKREPLYPROXY_H
#define _NETWORKREPLYPROXY_H

#include <QObject>
#include <QNetworkReply>
#include <QTimer>

class NetworkReplyProxy : public QNetworkReply {
  Q_OBJECT

  public:
    NetworkReplyProxy(QNetworkReply* reply, int timeout = 0, QObject* parent = NULL);

    void abort();
    void close();
    bool isSequential() const;
    qint64 bytesAvailable() const;
    qint64 readData(char* data, qint64 maxlen);
    int timerInterval() const;
    void setTimerInterval(int value);

  public slots:
    void ignoreSslErrors();

  private slots:
    void applyMetaData(bool signal = true);
    void errorInternal(QNetworkReply::NetworkError _error);
    void handleReadyRead(bool is_finished = false);
    void handleFinished();
    void timeout();

  private:
    QTimer m_timer;
    QNetworkReply* m_reply;
    QByteArray m_buffer;
};

#endif
