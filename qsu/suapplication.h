/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef SUAPPLICATION_H
#define SUAPPLICATION_H

#include <QApplication>
#include <QByteArray>

class SuProcessExecutor;

class SuApplication : public QApplication {
    Q_OBJECT
public:
    SuApplication(int &argc, char **argv);

private slots:
    void loopStarted();
    void oncompleted(int code);
    void onauthFailure();
    void errDlgDestroyed();
    void onreadyReadStandardError(const QByteArray & data);
    void onreadyReadStandardErrorLine(const QByteArray & data);
    void onreadyReadStandardOutput(const QByteArray & data);
    void onreadyReadStandardOutputLine(const QByteArray & data);

private:
    SuProcessExecutor * m_executor;
    bool m_was_error;
    int m_counter;
    QByteArray m_buffer;
};

#endif // SUAPPLICATION_H
