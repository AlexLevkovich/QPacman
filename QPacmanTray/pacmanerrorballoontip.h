/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANERRORBALLOONTIP_H
#define PACMANERRORBALLOONTIP_H

#include <QObject>

class SystemTrayIcon;

class PacmanErrorBalloonTip : public QObject {
    Q_OBJECT
public:
    explicit PacmanErrorBalloonTip(SystemTrayIcon *ti);
    ~PacmanErrorBalloonTip() {}
    void show();

signals:
    void showErrorsRequested();

private:
    QString message_str;
    SystemTrayIcon *ti;
};

#endif // PACMANERRORBALLOONTIP_H
