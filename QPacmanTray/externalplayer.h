/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef EXTERNALPLAYER_H
#define EXTERNALPLAYER_H

#include <QObject>
#include <QProcess>

class ExternalPlayer : public QObject {
    Q_OBJECT
public:
    explicit ExternalPlayer(const QString & file_name,QObject *parent = 0);
    ~ExternalPlayer();
    void play();

private slots:
    void player_started();

private:
    QString m_file_name;
    QProcess m_player;
};

#endif // EXTERNALPLAYER_H
