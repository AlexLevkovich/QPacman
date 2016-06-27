/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanprocessreader.h"
#include <QTimer>
#include <QCoreApplication>
#include <sys/types.h>
#include <signal.h>

PacmanProcessReader::PacmanProcessReader(QObject *parent) : QObject(parent) {
    code = 0;
    isTerminated = false;
    isFinished = false;

    connect(&process,SIGNAL(readyReadStandardError()),this,SLOT(readyReadStandardError()));
    connect(&process,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()));
    connect(&process,SIGNAL(error(QProcess::ProcessError)),this,SLOT(onError(QProcess::ProcessError)));
    connect(&process,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(onFinished(int,QProcess::ExitStatus)));

    QMetaObject::invokeMethod(this,"start",Qt::QueuedConnection);
}

PacmanProcessReader::~PacmanProcessReader() {
    terminate();
}

void PacmanProcessReader::waitToComplete() {
    QEventLoop loop;
    connect(this,SIGNAL(finished(PacmanProcessReader *)),&loop,SLOT(quit()));
    loop.exec();
}

void PacmanProcessReader::terminate() {
    if (process.state() == QProcess::Running) {
        QStringList list;
        Q_PID pid=process.pid();
        QProcess killer;
        QStringList params;
        params << "--ppid";
        params << QString::number(pid);
        params << "-o";
        params << "pid";
        params << "--noheaders";
        killer.start(QString("%1/ps").arg(TOOLS_BIN),params,QIODevice::ReadOnly);
        if(killer.waitForStarted(-1)) {
            if(killer.waitForFinished(-1)) {
                QByteArray temp=killer.readAllStandardOutput();
                QString str=QString::fromLocal8Bit(temp);
                list=str.split("\n");
            }
        }

        process.terminate();
        if (!process.waitForFinished()) process.kill();
        for(int i=0;i<list.size();i++) {
            if(!list.at(i).isEmpty()) ::kill(list.at(i).toInt(),SIGTERM);
        }
    }
    isTerminated = true;
}

void PacmanProcessReader::start() {
    if (code != 0) {
        QMetaObject::invokeMethod(this,"_finished",Qt::QueuedConnection);
        return;
    }

    process.setEnvironment(process.systemEnvironment() << "LANG=C");
    process.start(command());
}

void PacmanProcessReader::readyReadStandardError() {
    QString errStr = QString::fromLocal8Bit(process.readAllStandardError());
    m_errorStream += errStr;
    error(errStr);
}

QString PacmanProcessReader::errorStream() const {
    return ((PacmanProcessReader *)this)->m_errorStream;
}

void PacmanProcessReader::readyReadStandardOutput() {
    process.readAllStandardOutput();
}

void PacmanProcessReader::onError(QProcess::ProcessError errid) {
    switch (errid) {
        case QProcess::FailedToStart:
            error(tr("The process failed to start."));
            break;
        case QProcess::Crashed:
            error(tr("The process crashed some time after starting successfully."));
            break;
        case QProcess::Timedout:
            error(tr("The last waitFor...() function timed out."));
            break;
        case QProcess::WriteError:
            error(tr("An error occurred when attempting to write to the process."));
            break;
        case QProcess::ReadError:
            error(tr("An error occurred when attempting to read from the process."));
            break;
        default:
            error(tr("An unknown error occurred."));
            break;
    }
}

void PacmanProcessReader::onFinished(int /*code*/,QProcess::ExitStatus /*status*/) {
    if (isFinished) return;

    isFinished = true;
    process.setReadChannel(QProcess::StandardOutput);
    if (process.bytesAvailable() > 0) readyReadStandardOutput();
    process.setReadChannel(QProcess::StandardError);
    if (process.bytesAvailable() > 0) readyReadStandardError();

    QMetaObject::invokeMethod(this,"_finished",Qt::QueuedConnection);
}

void PacmanProcessReader::_finished() {
    if (isTerminated) code = 0;

    emit finished(this);
}

bool PacmanProcessReader::error(const QString & /*error*/) { return true; }

int PacmanProcessReader::exitCode() const {
    return process.exitCode() + code;
}
