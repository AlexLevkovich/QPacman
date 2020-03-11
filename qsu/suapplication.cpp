/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "suapplication.h"
#include "messagedialog.h"
#include "suprocessexecutor.h"
#include <QTimer>
#include <QDebug>

SuApplication::SuApplication(int &argc, char **argv) : QApplication(argc,argv) {
    m_was_error = false;
    m_counter = 3;

    QApplication::setQuitOnLastWindowClosed(false);
    QTimer::singleShot(0,this,SLOT(loopStarted()));
}

void SuApplication::loopStarted() {
    QStringList args = arguments();
    args.removeAt(0);

    m_executor = new SuProcessExecutor();
    connect(m_executor,SIGNAL(completed(int)),this,SLOT(oncompleted(int)),Qt::QueuedConnection);
    connect(m_executor,SIGNAL(authFailure()),this,SLOT(onauthFailure()));
    connect(m_executor,SIGNAL(readyReadStandardError(const QByteArray &)),this,SLOT(onreadyReadStandardError(const QByteArray &)));
    connect(m_executor,SIGNAL(readyReadStandardErrorLine(const QByteArray &)),this,SLOT(onreadyReadStandardErrorLine(const QByteArray &)));
    connect(m_executor,SIGNAL(readyReadStandardOutput(const QByteArray &)),this,SLOT(onreadyReadStandardOutput(const QByteArray &)));
    connect(m_executor,SIGNAL(readyReadStandardOutputLine(const QByteArray &)),this,SLOT(onreadyReadStandardOutputLine(const QByteArray &)));
    m_executor->setCommand(args);
}

void SuApplication::oncompleted(int code) {
    m_executor->deleteLater();
    if (!m_was_error || m_counter <= 0) {
        if (m_counter <= 0) {
            connect(ErrorDialog::post(tr("Authentication failure!"),""),SIGNAL(destroyed()),SLOT(errDlgDestroyed()));
            return;
        }
        exit(code);
        return;
    }
    m_was_error = false;
    QTimer::singleShot(0,this,SLOT(loopStarted()));
}

void SuApplication::errDlgDestroyed() {
    exit(127);
}

void SuApplication::onauthFailure() {
    m_was_error = true;
    m_counter--;
}

void SuApplication::onreadyReadStandardError(const QByteArray &data)  {
    fprintf(stderr,"%s",data.constData());
}

void SuApplication::onreadyReadStandardErrorLine(const QByteArray & data) {
    fprintf(stderr,"%s\n",data.constData());
}

void SuApplication::onreadyReadStandardOutput(const QByteArray & data) {
    fprintf(stdout,"%s",data.constData());
}

void SuApplication::onreadyReadStandardOutputLine(const QByteArray & data) {
    fprintf(stdout,"%s\n",data.constData());
}
