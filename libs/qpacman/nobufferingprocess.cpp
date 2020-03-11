/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "nobufferingprocess.h"
#include <QFileInfo>
#include "slottedeventloop.h"
#include <QTimer>
#include <QDir>
#include <QFileSystemWatcher>

NoBufferingProcess::NoBufferingProcess(QObject *parent) : QObject(parent) {
    init();
}

NoBufferingProcess::NoBufferingProcess(const QString & pgm_name,const QStringList & args,QObject *parent) : QObject(parent) {
    init();

    setProgram(pgm_name);
    setArguments(args);
}

NoBufferingProcess::NoBufferingProcess(const QString & pgm_name_and_args,QObject *parent) : QObject(parent) {
    init();

    QStringList list = splitCommandLine(pgm_name_and_args);
    if (list.count() <= 0) return;

    setProgram(list.at(0));
    list.removeAt(0);
    if (list.count() <= 0) return;

    setArguments(list);
}

NoBufferingProcess::~NoBufferingProcess() {
    if (m_stdout.isOpen()) m_stdout.close();
    if (m_stderr.isOpen()) m_stderr.close();
    m_stdout.remove();
    m_stderr.remove();
}

void NoBufferingProcess::init() {
    m_latest_err_pos = 0;
    m_latest_out_pos = 0;
    m_error = UnknownError;
    m_watcher = NULL;

    m_stdout.setAutoRemove(false);
    m_stderr.setAutoRemove(false);
    m_stdout.setFileTemplate(QDir::tempPath()+QDir::separator()+"tmp_outXXXXXX");
    m_stderr.setFileTemplate(QDir::tempPath()+QDir::separator()+"tmp_errXXXXXX");

    connect(&m_process,SIGNAL(errorOccurred(QProcess::ProcessError)),this,SLOT(onerrorOccurred(QProcess::ProcessError)));
    connect(&m_process,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(onfinished(int,QProcess::ExitStatus)));
    connect(&m_process,SIGNAL(started()),this,SLOT(onstarted()));
    connect(&m_process,SIGNAL(stateChanged(QProcess::ProcessState)),this,SLOT(onstateChanged(QProcess::ProcessState)));
}

void NoBufferingProcess::onfinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QByteArray bytes;
    while (readStdoutBytes(bytes)) {
        processBytes(bytes,true);
    }
    while (readStderrBytes(bytes)) {
        processBytes(bytes,false);
    }
    m_latest_err_line.clear();
    m_latest_out_line.clear();
    m_latest_err_pos = 0;
    m_latest_out_pos = 0;
    if (m_watcher != NULL) delete m_watcher;
    m_watcher = NULL;
    m_stdout.remove();
    m_stderr.remove();

    emit finished(exitCode,(NoBufferingProcess::ExitStatus)exitStatus);
}

void NoBufferingProcess::onerrorOccurred(QProcess::ProcessError error) {
    emit errorOccurred((NoBufferingProcess::ProcessError)error);
}

void NoBufferingProcess::onstateChanged(QProcess::ProcessState newState) {
    emit stateChanged((NoBufferingProcess::ProcessState)newState);
}

QStringList NoBufferingProcess::arguments() const {
    return m_process.arguments();
}

void NoBufferingProcess::setArguments(const QStringList & arguments) {
    m_process.setArguments(arguments);
}

QString NoBufferingProcess::program() const {
    return m_process.program();
}

void NoBufferingProcess::setProgram(const QString & program) {
    m_process.setProgram(program);
}

qint64 NoBufferingProcess::processId() const {
    return m_process.processId();
}

QStringList NoBufferingProcess::splitCommandLine(const QString & cmdLine) {
    QStringList list;
    QString arg;
    bool escape = false;
    enum { Idle, Arg, QuotedArg } state = Idle;
    foreach (QChar const c, cmdLine) {
        if (!escape && c == '\\') { escape = true; continue; }
        switch (state) {
        case Idle:
            if (!escape && c == '"') state = QuotedArg;
            else if (escape || !c.isSpace()) { arg += c; state = Arg; }
            break;
        case Arg:
            if (!escape && c == '"') state = QuotedArg;
            else if (escape || !c.isSpace()) arg += c;
            else { list << arg; arg.clear(); state = Idle; }
            break;
        case QuotedArg:
            if (!escape && c == '"') state = arg.isEmpty() ? Idle : Arg;
            else arg += c;
            break;
        }
        escape = false;
    }
    if (!arg.isEmpty()) list << arg;
    return list;
}

void NoBufferingProcess::start() {
    if (!m_stdout.open()) {
        m_error = FailedCreateStdout;
        emit errorOccurred(m_error);
        return ;
    }
    if (!m_stderr.open()) {
        m_error = FailedCreateStderr;
        emit errorOccurred(m_error);
        return;
    }
    m_stdout_name = m_stdout.fileName();
    m_stderr_name = m_stderr.fileName();

    m_process.setStandardOutputFile(m_stdout_name);
    m_process.setStandardErrorFile(m_stderr_name);

    m_latest_err_line.clear();
    m_latest_out_line.clear();
    m_latest_err_pos = 0;
    m_latest_out_pos = 0;

    m_process.start();
}

void NoBufferingProcess::onstarted() {
    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher,SIGNAL(fileChanged(const QString &)),this,SLOT(onfileChanged(const QString &)));
    m_watcher->addPath(m_stdout_name);
    m_watcher->addPath(m_stderr_name);
    emit started();
}

QProcessEnvironment NoBufferingProcess::processEnvironment() const {
    return m_process.processEnvironment();
}

void NoBufferingProcess::setProcessEnvironment(const QProcessEnvironment &environment) {
    m_process.setProcessEnvironment(environment);
}

void NoBufferingProcess::setProcessEnvironment(const QStringList &env) {
    QProcessEnvironment environment;
    QStringList tokens;
    QString name;
    QString value;
    for (int i=0;i<env.count();i++) {
        tokens = env.at(i).split('=');
        if (tokens.count() < 2) continue;
        name = tokens.at(0);
        value.clear();
        for (int j=1;j<env.count();j++) {
            value += tokens.at(j) + "=";
        }
        value = value.trimmed();
        environment.insert(name,value);
    }
    m_process.setProcessEnvironment(environment);
}

NoBufferingProcess::ProcessError NoBufferingProcess::error() const {
    return (m_error != UnknownError)?(NoBufferingProcess::ProcessError)m_process.error():m_error;
}

int NoBufferingProcess::exitCode() const {
    return m_process.exitCode();
}

void NoBufferingProcess::setWorkingDirectory(const QString & dir) {
    m_process.setWorkingDirectory(dir);
}

NoBufferingProcess::ProcessState NoBufferingProcess::state() const {
    return (NoBufferingProcess::ProcessState)m_process.state();
}

bool NoBufferingProcess::waitForFinished(int msecs) {
    if (state() == NoBufferingProcess::NotRunning) return true;

    SlottedEventLoop loop;
    connect(this,SIGNAL(finished(int,NoBufferingProcess::ExitStatus)),&loop,SLOT(quit()));
    if (msecs > 0) QTimer::singleShot(msecs,&loop,SLOT(reject()));
    if (loop.exec()) emit errorOccurred(NoBufferingProcess::Timedout);

    return (state() == NoBufferingProcess::Running)?false:true;
}

bool NoBufferingProcess::waitForStarted(int msecs) {
    if (state() != NoBufferingProcess::Starting) return false;

    SlottedEventLoop loop;
    connect(this,SIGNAL(started()),&loop,SLOT(quit()));
    if (msecs > 0) QTimer::singleShot(msecs,&loop,SLOT(reject()));
    if (loop.exec()) emit errorOccurred(NoBufferingProcess::Timedout);

    return (state() == NoBufferingProcess::Running)?true:false;
}

QString NoBufferingProcess::workingDirectory() const {
    return m_process.workingDirectory();
}

void NoBufferingProcess::kill() {
    m_process.kill();
}

void NoBufferingProcess::terminate() {
    m_process.terminate();
}

qint64 NoBufferingProcess::write(const char *data, qint64 maxSize) {
    if (maxSize <= 0) {
        m_error = WriteError;
        emit errorOccurred(m_error);
        return -1;
    }

    qint64 shift = 0;
    qint64 written;
    while ((written = m_process.write(data+shift,maxSize-shift)) != -1) {
        shift += written;
        if (shift >= maxSize) return maxSize;
    }
    m_error = WriteError;
    emit errorOccurred(m_error);
    return (shift == 0)?-1:shift;
}

qint64 NoBufferingProcess::write(const char *data) {
    return write(data,strlen(data));
}

qint64 NoBufferingProcess::write(const QByteArray &byteArray) {
    return write(byteArray.constData());
}

bool NoBufferingProcess::readStdoutBytes(QByteArray & bytes) {
    if (m_latest_out_pos >= QFileInfo(m_stdout_name).size()) return false;
    bytes.clear();
    QFile file(m_stdout_name);
    if (!file.open(QIODevice::ReadOnly)) {
        m_error = ReadError;
        emit errorOccurred(m_error);
        return false;
    }
    file.seek(m_latest_out_pos);
    bytes = file.readAll();
    m_latest_out_pos += bytes.count();
    return true;
}

bool NoBufferingProcess::readStderrBytes(QByteArray & bytes) {
    if (m_latest_err_pos >= QFileInfo(m_stderr_name).size()) return false;
    bytes.clear();
    QFile file(m_stderr_name);
    if (!file.open(QIODevice::ReadOnly)) {
        m_error = ReadError;
        emit errorOccurred(m_error);
        return false;
    }
    file.seek(m_latest_err_pos);
    bytes = file.readAll();
    m_latest_err_pos += bytes.count();
    return true;
}

void NoBufferingProcess::processBytes(const QByteArray & bytes,bool stdout) {
    QList<QByteArray> lines = bytes.split('\n');
    QByteArray end;
    if (!bytes.endsWith("\n")) {
        end = lines.last();
        lines.removeLast();
    }
    for (int i=0;i<lines.count();i++) {
        if (stdout) {
            emit readyReadStandardOutputLine((i==0)?m_latest_out_line+lines.at(i):lines.at(i));
            if (i==0) m_latest_out_line.clear();
        }
        else {
            emit readyReadStandardErrorLine((i==0)?m_latest_err_line+lines.at(i):lines.at(i));
            if (i==0) m_latest_err_line.clear();
        }
    }
    if (!end.isNull()) {
        if (stdout) {
            m_latest_out_line += end;
            emit readyReadStandardOutput(end);
        }
        else {
            m_latest_err_line += end;
            emit readyReadStandardError(end);
        }
    }
}

void NoBufferingProcess::onfileChanged(const QString & filePath) {
    QByteArray bytes;
    if (QFileInfo(filePath).fileName().startsWith("qpacman_out")) {
        if (readStdoutBytes(bytes)) processBytes(bytes,true);
    }
    else {
        if (readStderrBytes(bytes)) processBytes(bytes,false);
    }
}

NoBufferingProcess::ExitStatus NoBufferingProcess::exitStatus() const {
    return (NoBufferingProcess::ExitStatus)m_process.exitStatus();
}

QString NoBufferingProcess::errorString() const {
    switch (error()) {
    case NoBufferingProcess::FailedToStart:
        return tr("The process was not able to start!");
    case NoBufferingProcess::Crashed:
        return tr("The process crashed!");
    case NoBufferingProcess::Timedout:
        return tr("The waiting has been timed out!");
    case NoBufferingProcess::WriteError:
        return tr("The process was not able to write data!");
    case NoBufferingProcess::ReadError:
        return tr("The process was not able to read data!");
    case NoBufferingProcess::UnknownError:
        return "";
    case NoBufferingProcess::FailedCreateStderr:
        return tr("The creation of the temporary stderr file has been failed!");
    case NoBufferingProcess::FailedCreateStdout:
        return tr("The creation of the temporary stdout file has been failed!");
    }
    return "";
}
