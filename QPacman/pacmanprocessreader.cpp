/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanprocessreader.h"
#include <QTimer>
#include <QCoreApplication>
#include <sys/types.h>
#include <signal.h>
#include <sys/prctl.h>
#include <QDebug>
#include <QFile>
#ifdef IS_QPACMAN_CLIENT
#include "posterrordlg.h"
#endif
#include "confsettings.h"
#include "pacmansetupinforeader.h"

bool PacmanProcessReader::set_info_has_read = false;

class PacmanProcess: public QProcess {
    Q_OBJECT
public:
    PacmanProcess(QObject* parent = 0) : QProcess(parent) {}

protected:
    void setupChildProcess() {
        ::prctl(PR_SET_PDEATHSIG, SIGKILL);
        ::setbuf(stdout,NULL);
        ::setbuf(stderr,NULL);
    }
};

class RootChildProcessKiller : public PacmanProcessReader {
    Q_OBJECT
public:
    RootChildProcessKiller(const QString & su_password,qint64 pid,QObject *parent = 0) : PacmanProcessReader(su_password,parent) {
        m_pid = pid;
    }

protected:
    QString command() const {
        return QString("%1 -9 `%2 -p %3 | %4 '(' '\n' | %5 -v arg=')' '{ind=index($0,arg);if (ind > 0) {print substr($0,1,ind-1)}}'`;%6 -f %7").arg(KILL_BIN).arg(PSTREE_BIN).arg(m_pid).arg(TR_BIN).arg(AWK_BIN).arg(RM_BIN).arg(PacmanSetupInfoReader::pacman_lock_file);
    }

private:
    qint64 m_pid;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PacmanProcessReader::PacmanProcessReader(QObject *parent) : QObject(parent) {
    init();
}

PacmanProcessReader::PacmanProcessReader(const QString & su_password,QObject *parent) : QObject(parent) {
    init();
    m_su_password = su_password;
}

void PacmanProcessReader::init() {
    m_code = 0;
    isTerminated = false;
    isFinished = false;
    m_su_completed = true;

    process = new PacmanProcess(this);

    if (!set_info_has_read) {
        set_info_has_read = true;
        PacmanSetupInfoReader setup_reader;
        setup_reader.waitToComplete();
        if (setup_reader.exitCode() != 0) {
            QCoreApplication::instance()->quit();
            return;
        }
    }

    connect(process,SIGNAL(readyReadStandardError()),this,SLOT(readyReadStandardError()));
    connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()));
#if QT_VERSION >= 0x050000
    connect(process,SIGNAL(errorOccurred(QProcess::ProcessError)),SLOT(onError(QProcess::ProcessError)));
#else
    connect(process,SIGNAL(error(QProcess::ProcessError)),SLOT(onError(QProcess::ProcessError)));
#endif
    connect(process,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(onFinished(int,QProcess::ExitStatus)));

    QMetaObject::invokeMethod(this,"start",Qt::QueuedConnection);
}

PacmanProcessReader::~PacmanProcessReader() {
    terminate();
}

bool PacmanProcessReader::use_su() const {
    return !m_su_password.isEmpty();
}

void PacmanProcessReader::waitToComplete() {
    QEventLoop loop;
    connect(this,SIGNAL(finished(PacmanProcessReader *)),&loop,SLOT(quit()));
    loop.exec();
}

void PacmanProcessReader::terminate() {
    if (process->state() != QProcess::Running) return;

    RootChildProcessKiller(m_su_password,process->pid()).waitToComplete();

    isTerminated = true;
}

void PacmanProcessReader::start() {
    if (m_code != 0) {
        QMetaObject::invokeMethod(this,"_finished",Qt::QueuedConnection);
        return;
    }

    m_su_completed = !use_su();
    process->setEnvironment(process->systemEnvironment() << "LANG=C");
    QStringList args;
    args << "-c";
    if (use_su()) args << BASH_BIN << "-c";
    args << command();
    process->start(use_su()?SU_BIN:BASH_BIN,args);
}

void PacmanProcessReader::readyReadStandardError() {
    QString errStr = QString::fromLocal8Bit(process->readAllStandardError());
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
        if (!m_su_completed && errorLines.last().simplified() == "Password:") {
            m_su_completed = true;
            write(m_su_password.toLocal8Bit()+'\n');
        }
        if (!error(errorLines.last())) m_lastErrorStream += errorLines.last();
    }
}

QString PacmanProcessReader::errorStream() const {
    return ((PacmanProcessReader *)this)->m_errorStream;
}

void PacmanProcessReader::readyReadStandardOutput() {
    QString outStr = m_lastOutputStream + QString::fromLocal8Bit(process->readAllStandardOutput());
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
            m_errorStream += tr("The process failed to start.")+"\n";
            break;
        case QProcess::Crashed:
            m_errorStream += tr("The process crashed some time after starting successfully.")+"\n";
            break;
        case QProcess::Timedout:
            m_errorStream += tr("The last waitFor...() function timed out.")+"\n";
            break;
        case QProcess::WriteError:
            m_errorStream += tr("An error occurred when attempting to write to the process->")+"\n";
            break;
        case QProcess::ReadError:
            m_errorStream += tr("An error occurred when attempting to read from the process->")+"\n";
            break;
        default:
            m_errorStream += tr("An unknown error occurred.")+"\n";
            break;
    }
}

void PacmanProcessReader::onFinished(int /*code*/,QProcess::ExitStatus /*status*/) {
    if (isFinished) return;

    isFinished = true;
    if (process->bytesAvailable() > 0) readyReadStandardOutput();
    if (process->bytesAvailable() > 0) readyReadStandardError();

    QMetaObject::invokeMethod(this,"_finished",Qt::QueuedConnection);
}

void PacmanProcessReader::_finished() {
    if (isTerminated) m_code = 0;

    if (!isTerminated && exitCode() != 0) {
    #ifndef IS_QPACMAN_CLIENT
        emit was_error(errorStream(),command());
    #else
        new PostErrorDlg(errorStream(),command());
    #endif
    }

    emit finished(this);
}

bool PacmanProcessReader::error(const QString & /*err*/) { return true; }
bool PacmanProcessReader::output(const QString & /*out*/) { return true; }

int PacmanProcessReader::exitCode() const {
    return process->exitCode() + m_code;
}

#include "pacmanprocessreader.moc"
