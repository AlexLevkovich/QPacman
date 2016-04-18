/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanprocessreader.h"
#include <QMessageBox>
#ifdef IS_QPACMAN_CLIENT
#include "posterrordlg.h"
#endif
#include <QApplication>
#include "dbuswatcher.h"
#include "pacmanserverinterface.h"

PacmanProcessReader::PacmanProcessReader(QObject *parent) : QObject(parent) {
    code = 0;
    isFinished = false;

    connect(PacmanServerInterface::instance(),SIGNAL(command_finished(const QByteArray &,const QString &)),this,SLOT(onCommandFinished(const QByteArray &,const QString &)));
    connect(PacmanServerInterface::watcher(),SIGNAL(unloaded()),this,SLOT(_finish()));
    connect(PacmanServerInterface::instance(),SIGNAL(dbus_error(const QString &)),this,SLOT(error(const QString &)),Qt::QueuedConnection);

    QMetaObject::invokeMethod(this,"_start",Qt::QueuedConnection);
}

PacmanProcessReader::~PacmanProcessReader() {
    if (!isFinished) terminate();
}

void PacmanProcessReader::_start() {
    send_parameters();
    m_command = command();
    QString err_str = PacmanServerInterface::instance()->commandRequest(m_command);
    if (!err_str.isEmpty()) {
        code = 1;
        QMetaObject::invokeMethod(this,"error",Qt::QueuedConnection,Q_ARG(QString,err_str));
        QMetaObject::invokeMethod(this,"_finish",Qt::QueuedConnection);
    }
}

void PacmanProcessReader::_finish() {
    isFinished = true;
    emit finished(this);
}

bool PacmanProcessReader::isFinishedCommandCorrect(const QByteArray & command) {
    return (command == m_command);
}

void PacmanProcessReader::onCommandFinished(const QByteArray & command,const QString &errorMsg) {
    if (!isFinishedCommandCorrect(command)) return;

    if (!errorMsg.isEmpty()) {
        code = 1;
        QMetaObject::invokeMethod(this,"error",Qt::QueuedConnection,Q_ARG(QString,errorMsg));
    }
    QMetaObject::invokeMethod(this,"_finish",Qt::QueuedConnection);
}

void PacmanProcessReader::waitToComplete() {
    if (isFinished) return;

    QEventLoop loop;
    connect(this,SIGNAL(finished(PacmanProcessReader *)),&loop,SLOT(quit()));
    loop.exec();
}

void PacmanProcessReader::terminate() {
    if (!m_command.isEmpty()) PacmanServerInterface::instance()->terminateRequest(m_command);
}

void PacmanProcessReader::error(const QString & error) {
#ifndef IS_QPACMAN_CLIENT
    emit was_error(error,QString::fromLocal8Bit(m_command));
#else
    new PostErrorDlg(error,QString::fromLocal8Bit(m_command));
#endif
}

