/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "externalplayer.h"
#include <QFile>

#define BUFFER_LEN 1024

ExternalPlayer::ExternalPlayer(const QString & file_name,QObject *parent) : QObject(parent) {
    m_file_name = file_name;
    connect(&m_player,SIGNAL(started()),this,SLOT(player_started()));
}

ExternalPlayer::~ExternalPlayer() {
    if (m_player.state() == QProcess::Running) m_player.kill();
}

void ExternalPlayer::play() {
    if (m_player.state() == QProcess::Running) return;

    m_player.start("/usr/bin/ogg123 -");
}

void ExternalPlayer::player_started() {
    QFile input_file(m_file_name);
    if (!input_file.open(QIODevice::ReadOnly)) return;

    while (!input_file.atEnd()) m_player.write(input_file.read(BUFFER_LEN));

    input_file.close();
    m_player.closeWriteChannel();
}
