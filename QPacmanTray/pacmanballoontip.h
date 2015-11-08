/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANBALLOONTIP_H
#define PACMANBALLOONTIP_H

#include <QObject>

class SystemTrayIcon;

class PacmanBalloonTip : public QObject {
    Q_OBJECT

public:
    explicit PacmanBalloonTip(const QStringList & packages,SystemTrayIcon *ti);
    ~PacmanBalloonTip() {}
    void show();

signals:
    void updateRequested();

private:
    QString message_str;
    SystemTrayIcon *ti;
};

#endif // PACMANBALLOONTIP_H
