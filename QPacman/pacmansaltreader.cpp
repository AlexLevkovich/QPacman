/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmansaltreader.h"
#include "pacmanserverinterface.h"

PacmanSaltReader::PacmanSaltReader(QObject *parent) : PacmanProcessReader(parent) {
    connect(PacmanServerInterface::instance(),SIGNAL(salt_ready(const QByteArray &)),this,SLOT(on_salt_ready(const QByteArray &)));
}

QByteArray PacmanSaltReader::command() const {
    return "SALT";
}

void PacmanSaltReader::send_parameters() {}

void PacmanSaltReader::on_salt_ready(const QByteArray & salt) {
    m_salt = salt;
}
