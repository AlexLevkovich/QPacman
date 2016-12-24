/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanprocessreader.h"
#include <QTimer>
#include <QCoreApplication>
#include <sys/types.h>
#include <signal.h>
#include <QDebug>
#include <QFile>

extern QString pacman_lock_file;

PacmanProcessReader::PacmanProcessReader(QObject *parent) : QObject(parent) {
    m_code = 0;
    isTerminated = false;
    isFinished = false;

    connect(&process,SIGNAL(readyReadStandardError()),this,SLOT(readyReadStandardError()));
    connect(&process,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()));
#if QT_VERSION >= 0x050000
    connect(&process,SIGNAL(errorOccurred(QProcess::ProcessError)),SLOT(onError(QProcess::ProcessError)));
#else
    connect(&process,SIGNAL(error(QProcess::ProcessError)),SLOT(onError(QProcess::ProcessError)));
#endif    
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
        killer.start(PS_BIN,params,QIODevice::ReadOnly);
        if(killer.waitForStarted(-1)) {
            if(killer.waitForFinished(-1)) {
                QByteArray temp=killer.readAllStandardOutput();
                QString str=QString::fromLocal8Bit(temp);
                list=str.split("\n");
            }
        }

        terminateProcess();
        if (!process.waitForFinished()) process.kill();
        for(int i=0;i<list.size();i++) {
            if(!list.at(i).isEmpty()) ::kill(list.at(i).toInt(),SIGTERM);
        }
    }
    QFile::remove(pacman_lock_file);
    isTerminated = true;
}

void PacmanProcessReader::start() {
    if (m_code != 0) {
        QMetaObject::invokeMethod(this,"_finished",Qt::QueuedConnection);
        return;
    }

    process.setEnvironment(process.systemEnvironment() << "LANG=C");
    process.start(command());
}

void PacmanProcessReader::readyReadStandardError() {
    QString errStr = QString::fromLocal8Bit(process.readAllStandardError());
    m_errorStream += errStr;
    errStr = m_lastErrorStream + errStr;
    m_lastErrorStream.clear();

    QStringList errorLines = errStr.split("\n",QString::SkipEmptyParts);

    QString err;
    for (int i=0;i<(errorLines.count()-1);i++) {
        err = errorLines.at(i);
        err.remove('\r');
        error(err);
    }

    if (errorLines.count() > 0) {
        if (!error(errorLines.last())) m_lastErrorStream += errorLines.last();
    }
}

QString PacmanProcessReader::errorStream() const {
    return ((PacmanProcessReader *)this)->m_errorStream;
}

void PacmanProcessReader::readyReadStandardOutput() {
    QString outStr = m_lastOutputStream + QString::fromLocal8Bit(process.readAllStandardOutput());
    m_lastOutputStream.clear();

    QStringList outputLines = outStr.split("\n",QString::KeepEmptyParts);
    if (outputLines.count() > 0) {
        if (!outputLines.last().isEmpty()) m_lastOutputStream += outputLines.last();
        outputLines.removeAt(outputLines.count()-1);
    }

    QString out;
    for (int i=0;i<outputLines.count();i++) {
        out = outputLines.at(i);
        out.remove('\r');
        output(out);
    }
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
    if (process.bytesAvailable() > 0) readyReadStandardOutput();
    if (process.bytesAvailable() > 0) readyReadStandardError();

    QMetaObject::invokeMethod(this,"_finished",Qt::QueuedConnection);
}

void PacmanProcessReader::_finished() {
    if (isTerminated) m_code = 0;

    emit finished(this);
}

bool PacmanProcessReader::error(const QString & /*err*/) { return true; }
bool PacmanProcessReader::output(const QString & /*out*/) { return true; }

int PacmanProcessReader::exitCode() const {
    return process.exitCode() + m_code;
}
