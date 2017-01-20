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

    isTerminated = true;
    RootChildProcessKiller(m_su_password,process->pid()).waitToComplete();
}

void PacmanProcessReader::start() {
    if (m_code != 0) {
        QMetaObject::invokeMethod(this,"_finished",Qt::QueuedConnection);
        return;
    }

    m_su_completed = !use_su();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QString so_preload = env.value("LD_PRELOAD");
    QString so_bin;
    if (QFile(SO_BIN1).exists()) so_bin = SO_BIN1;
    else if (QFile(SO_BIN2).exists()) so_bin = SO_BIN2;

    env.insert("LANG","C");
    if (!so_bin.isEmpty()) env.insert("LD_PRELOAD",QString("%2%1").arg(so_bin).arg(so_preload.isEmpty()?"":(so_preload+":")));
    process->setProcessEnvironment(env);

    QStringList args;
    args << "-c";
    if (use_su()) {
        if (!so_bin.isEmpty()) args << QString("LD_PRELOAD=%2%1 ").arg(so_bin).arg(so_preload.isEmpty()?"":(so_preload+":")) + BASH_BIN + " -c \"" + command() + "\"";
        else args << BASH_BIN << "-c" << command();
    }
    else args << command();
    process->start(use_su()?SU_BIN:BASH_BIN,args);
}

void PacmanProcessReader::readyReadStandardError() {
    if (isTerminated) return;

    QString line;
    for (int i=0;!isTerminated && availableErrorBytesCount() > 0;i++) {
        line = QString::fromLocal8Bit(readErrorLine());
        if (line.at(line.length()-1) == '\n') line.truncate(line.length()-1);
        if (!line.isEmpty() && line.at(line.length()-1) == '\r') line.truncate(line.length()-1);
        if (i == 0 && !m_lastErrorStream.isEmpty()) {
            line = m_lastErrorStream + line;
            m_lastErrorStream.clear();
        }
        if (availableErrorBytesCount() <= 0) {
            if (!m_su_completed && line.simplified() == "Password:") {
                m_su_completed = true;
                write(m_su_password.toLocal8Bit()+'\n');
            }
            if (!error(line)) m_lastErrorStream += line;
            else m_errorStream += line + '\n';
        }
        else {
            error(line);
            m_errorStream += line + '\n';
        }
    }
}

QString PacmanProcessReader::errorStream() const {
    return ((PacmanProcessReader *)this)->m_errorStream;
}

void PacmanProcessReader::readyReadStandardOutput() {
    if (isTerminated) return;

    if (!isTerminated && availableOutputBytesCount() <= 0 && !m_lastOutputStream.isEmpty()) {
        output(m_lastOutputStream);
        return;
    }

    QString line;
    bool hasEnd = false;

    for (int i=0;!isTerminated && availableOutputBytesCount() > 0;i++) {
        line = QString::fromLocal8Bit(readOutputLine());
        hasEnd = (line.at(line.length()-1) == '\n');
        if (hasEnd) line.truncate(line.length()-1);
        if (!line.isEmpty() && line.at(line.length()-1) == '\r') line.truncate(line.length()-1);
        if (i == 0 && !m_lastOutputStream.isEmpty()) {
            line = m_lastOutputStream + line;
            m_lastOutputStream.clear();
        }
        if (availableOutputBytesCount() <= 0 && !hasEnd) {
            m_lastOutputStream += line;
            break;
        }
        output(line);
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
    if (!isTerminated) readyReadStandardOutput();
    if (!isTerminated) readyReadStandardError();

    if (isTerminated) m_code = -process->exitCode();

    QMetaObject::invokeMethod(this,"_finished",Qt::QueuedConnection);
}

void PacmanProcessReader::_finished() {
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

quint64 PacmanProcessReader::availableOutputBytesCount() {
    QProcess::ProcessChannel tmp = process->readChannel();
    process->setReadChannel(QProcess::StandardOutput);
    quint64 ret = process->bytesAvailable();
    process->setReadChannel(tmp);

    return ret;
}

quint64 PacmanProcessReader::availableErrorBytesCount() {
    QProcess::ProcessChannel tmp = process->readChannel();
    process->setReadChannel(QProcess::StandardError);
    quint64 ret = process->bytesAvailable();
    process->setReadChannel(tmp);

    return ret;
}

void PacmanProcessReader::waitForReadyRead(QProcess::ProcessChannel channel) {
    if (isTerminated) return;

    QProcess::ProcessChannel tmp = process->readChannel();
    process->setReadChannel(channel);
    process->waitForReadyRead(-1);
    process->setReadChannel(tmp);
}

QByteArray PacmanProcessReader::readOutputLine(qint64 maxSize) {
    QProcess::ProcessChannel tmp = process->readChannel();
    process->setReadChannel(QProcess::StandardOutput);
    QByteArray ret = process->readLine(maxSize);
    process->setReadChannel(tmp);

    return ret;
}

QByteArray PacmanProcessReader::readErrorLine(qint64 maxSize) {
    QProcess::ProcessChannel tmp = process->readChannel();
    process->setReadChannel(QProcess::StandardError);
    QByteArray ret = process->readLine(maxSize);
    process->setReadChannel(tmp);

    return ret;
}

#include "pacmanprocessreader.moc"
