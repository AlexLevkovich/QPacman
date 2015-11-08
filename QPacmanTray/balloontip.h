/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef BALLOONTIP_H
#define BALLOONTIP_H

#include <QWidget>
#include <QUrl>
#include "systemtrayicon.h"


class BalloonTip : public QWidget {
    Q_OBJECT
public:
    static BalloonTip * showBalloon(SystemTrayIcon::MessageIcon icon, const QString& title,
                                    const QString& msg, SystemTrayIcon *trayIcon,
                                    const QPoint& pos, int timeout, bool showArrow = true);
    static void hideBalloon();
    static bool isBalloonVisible();

private:
    BalloonTip(SystemTrayIcon::MessageIcon icon, const QString& title,
               const QString& msg, SystemTrayIcon *trayIcon);
    ~BalloonTip();
    void balloon(const QPoint&, int, bool);

signals:
    void anchorClicked(const QUrl &);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *e);
    void timerEvent(QTimerEvent *e);
    void showEvent(QShowEvent *e);

private:
    SystemTrayIcon *trayIcon;
    QPixmap pixmap;
    int timerId;
};

#endif // BALLOONTIP_H
