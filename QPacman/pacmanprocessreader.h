/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanProcessReader_H
#define PacmanProcessReader_H

#include <QObject>

class PacmanProcessReader : public QObject {
    Q_OBJECT
public:
    explicit PacmanProcessReader(QObject *parent = 0);
    ~PacmanProcessReader();
    virtual void terminate();
    void waitToComplete();
    int exitCode() { return code; }

signals:
    void finished(PacmanProcessReader * pointer);
#ifndef IS_QPACMAN_CLIENT
    void was_error(const QString & error,const QString & command);
#endif

protected slots:
    virtual void onCommandFinished(const QByteArray & command,const QString &errorMsg);
    virtual void error(const QString & error);

protected:
    virtual QByteArray command() const = 0;
    virtual void send_parameters() = 0;

private slots:
    void _start();
    void _finish();

private:
    int code;
    bool isFinished;
    QByteArray m_command;
};

#endif // PacmanProcessReader_H
