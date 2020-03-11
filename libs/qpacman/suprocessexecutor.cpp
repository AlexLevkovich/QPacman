/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "suprocessexecutor.h"
#include "rootdialog.h"
#include <QTemporaryFile>
#include <QSaveFile>
#include <QDir>
#include <QApplication>
#include <QStyle>
#include <QSettings>
#include <QDebug>

SuProcessExecutor::SuProcessExecutor() : QObject(NULL) {
    m_completed = false;
    no_reading_err_before = true;
    m_password_counter = 0;
}

SuProcessExecutor::SuProcessExecutor(const QStringList & cmd) : QObject(NULL) {
    m_completed = false;
    no_reading_err_before = true;
    m_cmd = cmd;
    m_password_counter = 0;
    QMetaObject::invokeMethod(this,"process",Qt::QueuedConnection);
}

SuProcessExecutor::~SuProcessExecutor() {
    QFile(m_temp_script).remove();
}

void SuProcessExecutor::setCommand(const QStringList & cmd) {
    m_cmd = cmd;
    QMetaObject::invokeMethod(this,"process",Qt::QueuedConnection);
}

int SuProcessExecutor::waitToComplete() {
    m_process.waitForFinished();
    return m_process.exitCode();
}

void SuProcessExecutor::process() {
    if (!createTempScript()) {
        m_error = tr("Cannot create the temporary script!");
        emit completed(127);
        return;
    }

    connect(&m_process,SIGNAL(finished(int,NoBufferingProcess::ExitStatus)),this,SLOT(onfinished(int,NoBufferingProcess::ExitStatus)));
    connect(&m_process,SIGNAL(errorOccurred(NoBufferingProcess::ProcessError)),this,SLOT(onerrorOccurred(NoBufferingProcess::ProcessError)));
    connect(&m_process,SIGNAL(readyReadStandardError(const QByteArray &)),this,SLOT(onreadyReadStandardError(const QByteArray &)));
    connect(&m_process,SIGNAL(readyReadStandardErrorLine(const QByteArray &)),this,SLOT(onreadyReadStandardErrorLine(const QByteArray &)));
    connect(&m_process,SIGNAL(readyReadStandardOutput(const QByteArray &)),this,SIGNAL(readyReadStandardOutput(const QByteArray &)));
    connect(&m_process,SIGNAL(readyReadStandardOutputLine(const QByteArray &)),this,SIGNAL(readyReadStandardOutputLine(const QByteArray &)));
    connect(&m_process,SIGNAL(readyReadStandardError(const QByteArray &)),this,SIGNAL(readyReadStandardError(const QByteArray &)));
    connect(&m_process,SIGNAL(readyReadStandardErrorLine(const QByteArray &)),this,SIGNAL(readyReadStandardErrorLine(const QByteArray &)));

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","C");
    m_process.setProcessEnvironment(env);
    m_process.setProgram(SU_BIN);
    m_process.setArguments(QStringList() << "-c" << m_temp_script << "-");
    m_process.start();
}

QString SuProcessExecutor::error() {
    return m_error.isEmpty()?m_process.errorString():m_error;
}

void SuProcessExecutor::onfinished(int exitCode,NoBufferingProcess::ExitStatus) {
    QFile(m_temp_script).remove();
    if (!m_completed) emit completed(exitCode);
}

void SuProcessExecutor::onerrorOccurred(NoBufferingProcess::ProcessError error) {
    QFile(m_temp_script).remove();
    if (error == NoBufferingProcess::FailedToStart && !m_completed) emit completed(127);
}

void SuProcessExecutor::onreadyReadStandardError(const QByteArray & data) {
    if (no_reading_err_before && (data.simplified() == "Password:")) {
        m_password_counter++;
        QString pw = getPassword();
        if (pw.isEmpty()) m_process.terminate();
        else m_process.write(pw.toLocal8Bit()+"\n");
    }
    else {
        no_reading_err_before = false;
        emit readyReadStandardError(data);
    }
}

void SuProcessExecutor::onreadyReadStandardErrorLine(const QByteArray & data) {
    if (no_reading_err_before && data.endsWith("Password: su: Authentication failure")) {
        emit authFailure();
    }
    else {
        if (no_reading_err_before) emit readyReadStandardErrorLine(data.mid(m_password_counter*10));
        else emit readyReadStandardErrorLine(data);
        no_reading_err_before = false;
    }
}

QString SuProcessExecutor::getPassword() const {
    RootDialog root_dlg;
    return (root_dlg.exec() == QDialog::Accepted)?root_dlg.password():QString();
}

bool SuProcessExecutor::createTempScript() {
    QTemporaryFile temp_file(QDir::tempPath()+QDir::separator()+"qpacman_temp_scriptXXXXXX");
    temp_file.setAutoRemove(false);
    if (!temp_file.open()) return false;
    m_temp_script = temp_file.fileName();
    temp_file.close();
    QSaveFile file(m_temp_script);
    if (!file.open(QIODevice::WriteOnly)) return false;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if ((file.write(QString("#!%1\n").arg(BASH_BIN).toLocal8Bit()) == -1) ||
        (file.write(QString("export LANG=%1\n").arg(env.value("LANG","C")).toLocal8Bit()) == -1) ||
        (file.write(QString("export DISPLAY=%1\n").arg(env.value("DISPLAY",":0")).toLocal8Bit()) == -1) ||
        (file.write(QString("export QT_STYLE_OVERRIDE=%1\n").arg(QApplication::style()->objectName()).toLocal8Bit()) == -1) ||
        (file.write(QString("export ORIGINAL_FONT=\"%1\"\n").arg(QApplication::font().toString()).toLocal8Bit()) == -1) ||
        (file.write(QString("export ORIGINAL_USER=%1\n").arg(env.value("USER","")).toLocal8Bit()) == -1) ||
        (file.write(QString("export ORIGINAL_DISPLAY=%1\n").arg(env.value("DISPLAY","")).toLocal8Bit()) == -1) ||
        (file.write(QString("export XAUTHORITY=%1%2%3\n").arg(QDir::homePath()).arg(QDir::separator()).arg(".Xauthority").toLocal8Bit()) == -1) ||
        (file.write(QString("export QPACMAN_SYNC_DIR=%1%2sync\n").arg(QFileInfo(QSettings().fileName()).dir().path()).arg(QDir::separator()).toLocal8Bit()) == -1) ||
        (file.write(QString("export XDG_CURRENT_DESKTOP=%1\n").arg(env.value("XDG_CURRENT_DESKTOP","")).toLocal8Bit()) == -1) ||
#ifdef QT_DEBUG
        (env.value("LD_LIBRARY_PATH","").isEmpty()?false:(file.write(QString("export LD_LIBRARY_PATH=%1\n").arg(env.value("LD_LIBRARY_PATH","")).toLocal8Bit()) == -1)) ||
#endif
        (file.write(QString("cd %1\n").arg(QDir::currentPath()).toLocal8Bit()) == -1) ||
        (file.write(QString("%1\n").arg(command()).toLocal8Bit()) == -1) ||
        !file.commit()) {
        file.cancelWriting();
        QFile(file.fileName()).remove();
        return false;
    }
    QFile(file.fileName()).setPermissions((QFileDevice::Permissions)0x5555);

    return true;
}

QString SuProcessExecutor::command() const {
    QString ret;
    QString arg;
    for (int i=0;i<m_cmd.count();i++) {
        arg = m_cmd.at(i);
        if (arg.toStdString().find_first_of(" \t") != std::string::npos) {
            arg = "\"" + arg + "\"";
        }
        ret += arg + " ";
    }
    if (ret.length() > 0) ret = ret.mid(0,ret.length()-1);
    return ret;
}
