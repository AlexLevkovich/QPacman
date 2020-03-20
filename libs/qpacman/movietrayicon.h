/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef MOVIETRAYICON_H
#define MOVIETRAYICON_H

#include <QList>
#include <QIcon>
#include <QMenu>
#include "movieicon.h"
#include <QSystemTrayIcon>

class QEvent;

class MovieTrayIcon : public QObject {
    Q_OBJECT
public:
    MovieTrayIcon(QObject *parent = nullptr);
    ~MovieTrayIcon();
    void setVisible(bool flag);
    void show();
    void hide();
    bool isVisible() const;
    void setIcon(const QIcon & icon);
    void setIcon(const QString & iconname);
    void setIcon(const QString & iconname,int input_frame_height,int delay);
    void showMessage(const QString & title,const QString & msg,const QIcon & icon,int msecs = 10000);
    void showMessage(const QString & title,const QString & msg,QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,int msecs = 10000);
    void setToolTip(const QString & title,const QString & msg);
    void clearTooltip();
    QRect geometry() const;

private slots:
    void frameChanged(int id);
    void activatedReason(QSystemTrayIcon::ActivationReason reason);
    void setMenu();
    void setTrayVisible(bool flag);
    void checkTopLevelWindows();

protected:
    virtual void initMenu(QMenu * menu);
    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void messageClicked();
    void fillingMenuRequest(QMenu * menu);
    void doubleClicked();
    void clicked();
    void middleClicked();
    void menuAboutToShow(QMenu * menu);

private:
    QStringList truncateMsg(const QString & msg) const;

    QSystemTrayIcon * m_tray_icon;
    MovieIcon m_movie;
    QMenu * m_menu;
    bool first_time_menu_show;

    static const int MAX_LINE_MSG_COUNT;
};

#endif // MOVIETRAYICON_H
