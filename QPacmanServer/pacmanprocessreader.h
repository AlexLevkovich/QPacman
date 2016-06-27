/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanProcessReader_H
#define PacmanProcessReader_H

#include <QObject>
#include <QProcess>

class PacmanProcessReader : public QObject {
    Q_OBJECT
public:
    explicit PacmanProcessReader(QObject *parent = 0);
    ~PacmanProcessReader();
    virtual void terminate();
    int exitCode() const;
    QString errorStream() const;
    void waitToComplete();
    bool wasTerminated() { return isTerminated; }

signals:
    void finished(PacmanProcessReader * pointer);

protected slots:
    virtual void readyReadStandardError();
    virtual void readyReadStandardOutput();
    void onError(QProcess::ProcessError errid);
    virtual void onFinished(int code,QProcess::ExitStatus status);
    virtual void start();

protected:
    QProcess process;
    QString m_errorStream;
    int code;
    bool isFinished;
    bool isTerminated;

    virtual bool error(const QString & error);
    virtual QString command() const = 0;

private slots:
    void _finished();
};

#endif // PacmanProcessReader_H
