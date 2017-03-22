/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanProcessReader_H
#define PacmanProcessReader_H

#include <QObject>
#include <QProcess>
#include "libqpacman_global.h"

class LIBQPACMANSHARED_EXPORT PacmanProcessReader : public QObject {
    Q_OBJECT
public:
    PacmanProcessReader(QObject *parent = 0);
    PacmanProcessReader(const QString & su_password,QObject *parent = 0);
    ~PacmanProcessReader();

    virtual void terminate();
    int exitCode() const;
    QString errorStream() const;
    void waitToComplete();

signals:
    void finished(PacmanProcessReader * pointer);

protected slots:
    void onError(QProcess::ProcessError errid);
    virtual void onFinished(int code,QProcess::ExitStatus status);
    virtual void start();

protected:
    inline int code() { return m_code; }
    inline void setCode(int code) { m_code = code; }
    inline qint64 write(const QByteArray & arr) { return process->write(arr); }
    inline void addToErrorStreamCache(const QString & err) { m_errorStream += err; }
    inline void clearErrorStreamCache() { m_errorStream.clear(); }
    inline bool wasTerminated() { return isTerminated; }
    inline void setTerminated(bool flag) { isTerminated = flag; }
    inline bool wasFinished() { return isFinished; }
    inline void waitForBytesWritten() { process->waitForBytesWritten(-1); }
    void waitForReadyRead(QProcess::ProcessChannel channel = QProcess::StandardOutput);
    quint64 availableOutputBytesCount();
    quint64 availableErrorBytesCount();
    QByteArray readOutputLine(qint64 maxSize = 0);
    QByteArray readErrorLine(qint64 maxSize = 0);
    void setDebugOutput(bool flag);
    inline bool isDebugOutput() { return m_debug_output;}

    virtual bool error(const QString & err);
    virtual bool output(const QString & out);
    virtual QString command() const = 0;
    virtual bool use_su() const;
    virtual bool showErrorMessageAtExit();

    void waitForEmptyOutput();

private slots:
    void _finished();
    void readyReadStandardError();
    void readyReadStandardOutput();
    void _onFinished(int code,QProcess::ExitStatus status);

private:
    void init();

    QString m_errorStream;
    QString m_lastErrorStream;
    QString m_lastOutputStream;
    int m_code;
    QProcess * process;
    bool isFinished;
    bool isTerminated;
    QString m_su_password;
    bool m_su_completed;
    bool m_debug_output;

    static bool set_info_has_read;
};

#endif // PacmanProcessReader_H
