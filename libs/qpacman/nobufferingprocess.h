/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef NOBUFFERINGPROCESS_H
#define NOBUFFERINGPROCESS_H

#include <QProcess>
#include <QStringList>
#include <QTemporaryFile>

class QFileSystemWatcher;

class NoBufferingProcess : public QObject {
    Q_OBJECT
public:
    enum ProcessError {
        FailedToStart = QProcess::FailedToStart,
        Crashed = QProcess::Crashed,
        Timedout = QProcess::Timedout,
        WriteError = QProcess::WriteError,
        ReadError = QProcess::ReadError,
        UnknownError = QProcess::UnknownError,
        FailedCreateStderr,
        FailedCreateStdout
    };

    enum ProcessState {
        NotRunning = QProcess::NotRunning,
        Starting = QProcess::Starting,
        Running = QProcess::Running
    };

    enum ExitStatus {
        NormalExit = QProcess::NormalExit,
        CrashExit = QProcess::CrashExit
    };

    NoBufferingProcess(QObject *parent = NULL);
    NoBufferingProcess(const QString & pgm_name,const QStringList & args,QObject *parent = NULL);
    NoBufferingProcess(const QString & pgm_name_and_args,QObject *parent = NULL);
    ~NoBufferingProcess();

    QStringList arguments() const;
    void setArguments(const QStringList & arguments);
    QString program() const;
    void setProgram(const QString & program);
    void start();
    ProcessError error() const;
    QString errorString() const;
    int exitCode() const;
    ExitStatus exitStatus() const;
    qint64 processId() const;
    QString workingDirectory() const;
    void setWorkingDirectory(const QString & dir);
    ProcessState state() const;
    bool waitForFinished(int msecs = 0);
    bool waitForStarted(int msecs = 0);
    QProcessEnvironment processEnvironment() const;
    void setProcessEnvironment(const QProcessEnvironment &environment);
    void setProcessEnvironment(const QStringList &environment);

    qint64 write(const char *data, qint64 maxSize);
    qint64 write(const char *data);
    qint64 write(const QByteArray &byteArray);

    static QStringList splitCommandLine(const QString & cmdLine);

public slots:
    void kill();
    void terminate();

private slots:
    void onerrorOccurred(QProcess::ProcessError error);
    void onstateChanged(QProcess::ProcessState newState);
    void onfinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onstarted();
    void onfileChanged(const QString & filePath);

signals:
    void readyReadStandardOutput(const QByteArray & data);
    void readyReadStandardError(const QByteArray & data);
    void readyReadStandardOutputLine(const QByteArray & data);
    void readyReadStandardErrorLine(const QByteArray & data);
    void errorOccurred(NoBufferingProcess::ProcessError error);
    void finished(int exitCode, NoBufferingProcess::ExitStatus exitStatus);
    void started();
    void stateChanged(NoBufferingProcess::ProcessState newState);

private:
    void init();
    bool readStdoutBytes(QByteArray & bytes);
    bool readStderrBytes(QByteArray & bytes);
    void processBytes(const QByteArray & bytes,bool stdout);

    QProcess m_process;
    QTemporaryFile m_stdout;
    QTemporaryFile m_stderr;
    QString m_stdout_name;
    QString m_stderr_name;
    QByteArray m_latest_err_line;
    QByteArray m_latest_out_line;
    qint64 m_latest_err_pos;
    qint64 m_latest_out_pos;
    ProcessError m_error;
    QFileSystemWatcher * m_watcher;

};
Q_DECLARE_METATYPE(NoBufferingProcess::ProcessError)
Q_DECLARE_METATYPE(NoBufferingProcess::ProcessState)
Q_DECLARE_METATYPE(NoBufferingProcess::ExitStatus)
Q_DECLARE_METATYPE(QProcess::ExitStatus)

#endif // NOBUFFERINGPROCESS_H
