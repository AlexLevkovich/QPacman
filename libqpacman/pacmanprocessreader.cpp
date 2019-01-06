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
#include "errordialog.h"
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
    m_debug_output = false;

    process = new PacmanProcess(this);

    if (!set_info_has_read) {
        set_info_has_read = true;
        PacmanSetupInfoReader setup_reader;
        setup_reader.waitToComplete();
        if (setup_reader.exitCode() != 0) {
            qCritical() << setup_reader.errorStream();
            ::exit(1);
        }
    }

    connect(process,SIGNAL(readyReadStandardError()),this,SLOT(readyReadStandardError()));
    connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()));
#if QT_VERSION >= 0x050000
    connect(process,SIGNAL(errorOccurred(QProcess::ProcessError)),SLOT(onError(QProcess::ProcessError)));
#else
    connect(process,SIGNAL(error(QProcess::ProcessError)),SLOT(onError(QProcess::ProcessError)));
#endif
    connect(process,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(_onFinished(int,QProcess::ExitStatus)));

    QMetaObject::invokeMethod(this,"start",Qt::QueuedConnection);
}

PacmanProcessReader::~PacmanProcessReader() {
    terminate();
}

void PacmanProcessReader::setDebugOutput(bool flag) {
    m_debug_output = flag;
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

    QProcess rootkiller;
    rootkiller.start(QPACMANKILL_BIN,QStringList()<<QString("%1").arg(process->pid()));
    rootkiller.waitForFinished(-1);
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

    if (availableErrorBytesCount() <= 0) return;

    QString line = QString::fromLocal8Bit(readErrorLine());
    if (line.at(line.length()-1) == '\n') line.truncate(line.length()-1);
    if (!line.isEmpty() && line.at(line.length()-1) == '\r') line.truncate(line.length()-1);
    if (!m_lastErrorStream.isEmpty()) {
        line = m_lastErrorStream + line;
        m_lastErrorStream.clear();
    }
    if (availableErrorBytesCount() <= 0) {
        if (!m_su_completed && line.simplified() == "Password:") {
            m_su_completed = true;
            write(m_su_password.toLocal8Bit()+'\n');
            m_errorStream += line + '\n';
        }
        else {
            if (!error(line)) m_lastErrorStream += line;
            else m_errorStream += line + '\n';
        }
    }
    else {
        error(line);
        m_errorStream += line + '\n';
    }

    if (availableErrorBytesCount() > 0) readyReadStandardError();
}

QString PacmanProcessReader::errorStream() const {
    return ((PacmanProcessReader *)this)->m_errorStream;
}

void PacmanProcessReader::readyReadStandardOutput() {
    if (isTerminated) return;

    if (availableOutputBytesCount() <= 0) return;

    QString line = QString::fromLocal8Bit(readOutputLine());
    bool hasEnd = (line.at(line.length()-1) == '\n');
    if (hasEnd) line.truncate(line.length()-1);
    if (!line.isEmpty() && line.at(line.length()-1) == '\r') line.truncate(line.length()-1);
    if (!m_lastOutputStream.isEmpty()) {
        line = m_lastOutputStream + line;
        m_lastOutputStream.clear();
    }
    if (availableOutputBytesCount() <= 0 && !hasEnd) {
        m_lastOutputStream += line;
        QMetaObject::invokeMethod(this,"readyReadStandardOutput",Qt::QueuedConnection);
        return;
    }
    output(line);
    if (m_debug_output) m_errorStream += line + '\n';

    if (availableOutputBytesCount() > 0) readyReadStandardOutput();
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

void PacmanProcessReader::_onFinished(int code,QProcess::ExitStatus status) {
    if (!isTerminated) {
        while (availableOutputBytesCount() > 0 || availableErrorBytesCount() > 0) {
            qApp->processEvents();
        }
    }

    onFinished(code,status);
}

void PacmanProcessReader::onFinished(int /*code*/,QProcess::ExitStatus /*status*/) {
    if (isFinished) return;
    isFinished = true;

    if (isTerminated) m_code = -process->exitCode();

    QMetaObject::invokeMethod(this,"_finished",Qt::QueuedConnection);
}

void PacmanProcessReader::_finished() {
    if (!isTerminated && (exitCode() != 0) && showErrorMessageAtExit()) ErrorDialog::post(tr("Error(s) during executing of the command:\n%1").arg(command()),errorStream());

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

void PacmanProcessReader::waitForEmptyOutput() {
    while (availableOutputBytesCount() > 0) {
        qApp->processEvents();
    }
}

void PacmanProcessReader::waitForEmptyError() {
    while (availableErrorBytesCount() > 0) {
        qApp->processEvents();
    }
}

bool PacmanProcessReader::showErrorMessageAtExit() {
    return true;
}

#include "pacmanprocessreader.moc"
