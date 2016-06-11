/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanremovepackagesreader.h"
#include "static.h"
#include "pacmanserverinterface.h"

PacmanRemovePackagesReader::PacmanRemovePackagesReader(const QString & packages,bool withDeps,QObject *parent) : PacmanProcessReader(parent) {
    in_packages = packages;
    m_withDeps = withDeps;

    connect(PacmanServerInterface::instance(),SIGNAL(ready_to_process_remove(const QStringList &,double)),this,SLOT(on_readyToProcess(const QStringList &,double)));
    connect(PacmanServerInterface::instance(),SIGNAL(post_messages(const QString &,const QStringList &)),this,SIGNAL(post_messages(const QString &,const QStringList &)));
    connect(PacmanServerInterface::instance(),SIGNAL(start_removing(const QString &)),this,SIGNAL(start_removing(const QString &)));
}

QStringList PacmanRemovePackagesReader::packages() const {
    return m_packages;
}

QByteArray PacmanRemovePackagesReader::command() const {
    return m_withDeps?"READ REMOVE PACKAGES":"READ REMOVE PACKAGES NO DEPS";
}

void PacmanRemovePackagesReader::send_parameters() {
    PacmanServerInterface::instance()->setPassword(Static::encryptedPassword);
    PacmanServerInterface::instance()->setPackages(in_packages);
}

void PacmanRemovePackagesReader::on_readyToProcess(const QStringList & packages, double total_removed) {
    m_packages = packages;
    emit ready_to_process(total_removed);
}

void PacmanRemovePackagesReader::beginRemove() {
    PacmanServerInterface::instance()->beginRemove();
}

void PacmanRemovePackagesReader::cancelRemove() {
    PacmanServerInterface::instance()->cancelRemove();
}

bool PacmanRemovePackagesReader::isFinishedCommandCorrect(const QByteArray & command) {
    return (command == "READ REMOVE PACKAGES" || command == "READ REMOVE PACKAGES NO DEPS");
}
