/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef SUPROCESSEXECUTOR_H
#define SUPROCESSEXECUTOR_H

#include "nobufferingprocess.h"

class SuProcessExecutor : public QObject {
    Q_OBJECT
public:
    SuProcessExecutor();
    SuProcessExecutor(const QStringList & cmd);
    ~SuProcessExecutor();
    int waitToComplete();
    QString error();
    void setCommand(const QStringList & cmd);
    QString command() const;

signals:
    void completed(int retcode);
    void readyReadStandardError(const QByteArray & data);
    void readyReadStandardErrorLine(const QByteArray & data);
    void readyReadStandardOutput(const QByteArray & data);
    void readyReadStandardOutputLine(const QByteArray & data);
    void authFailure();

private slots:
    void process();
    void onfinished(int exitCode, NoBufferingProcess::ExitStatus exitStatus);
    void onerrorOccurred(NoBufferingProcess::ProcessError error);
    void onreadyReadStandardError(const QByteArray & data);
    void onreadyReadStandardErrorLine(const QByteArray & data);

private:
    QString getPassword() const;
    bool createTempScript();

    QStringList m_cmd;
    NoBufferingProcess m_process;
    bool m_completed;
    bool no_reading_err_before;
    QString m_temp_script;
    QString m_error;
    int m_password_counter;
};

#endif // SUPROCESSEXECUTOR_H
