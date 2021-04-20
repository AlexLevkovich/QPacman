/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef DOWNLOADERINTERFACE_H
#define DOWNLOADERINTERFACE_H

#include <QObject>
#include <QString>
#include <QNetworkProxy>
#include <QDateTime>
#include <QFileInfo>
#include <sys/types.h>
#include <utime.h>

class DownloaderInterface : public QObject {
    Q_OBJECT
public:
    virtual bool isStarted() = 0;
    virtual bool start() = 0;
    virtual QString errorString() = 0;
    virtual qint64 dataLength() = 0;
    virtual bool terminate() = 0;
    virtual QUrl url() = 0;
    virtual bool setUrl(const QUrl & url) = 0;
    virtual QString outputName() = 0;
    virtual bool setOutputName(const QString & outputName) = 0;
    virtual void setProxy(const QNetworkProxy & proxy) = 0;
    virtual QNetworkProxy proxy() const = 0;
    virtual int threadsCount() = 0;
    virtual int timeout() = 0;
    virtual void setTimeout(uint value) = 0;

    static bool setFileDate(const QString & filename,const QDateTime & date) {
        if (date.isNull()) return false;

        struct utimbuf buf;
        buf.actime = QFileInfo(filename).fileTime(QFileDevice::FileAccessTime).toTime_t();
        buf.modtime = date.toTime_t();
        return (!::utime(filename.toLocal8Bit().constData(),&buf));
    }

protected:
    virtual void setErrorString(const QString & error) = 0;
    virtual void setDataLength(qint64 size) = 0;

signals:
    void location_changed(const QUrl & url);
    void error_occured();
    void download_completed();
    void download_terminated();
    void data_length_is_known(qint64 len);
    void progress(qint64 downloaded,int percents,qint64 speed);

protected:
    bool invokeMethod(const char *member,QGenericArgument val0 = QGenericArgument(),QGenericArgument val1 = QGenericArgument(),QGenericArgument val2 = QGenericArgument(),QGenericArgument val3 = QGenericArgument(),QGenericArgument val4 = QGenericArgument(),QGenericArgument val5 = QGenericArgument(),QGenericArgument val6 = QGenericArgument(),QGenericArgument val7 = QGenericArgument(),QGenericArgument val8 = QGenericArgument(),QGenericArgument val9 = QGenericArgument()) {
        return QMetaObject::invokeMethod(this,member,Qt::QueuedConnection,val0,val1,val2,val3,val4,val5,val6,val7,val8,val9);
    }
};

#endif // DOWNLOADERINTERFACE_H
