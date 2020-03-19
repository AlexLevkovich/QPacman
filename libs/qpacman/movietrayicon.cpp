/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "movietrayicon.h"
#include <QMovie>
#include <QMenu>
#include <QApplication>
#include <QFileInfo>
#include <QTimer>
#include <QMouseEvent>
#include <QDebug>

const int MovieTrayIcon::MAX_LINE_MSG_COUNT = 6;

TrayMenu::TrayMenu() : QMenu(NULL) {
    qApp->installEventFilter(this);
    connect(this,SIGNAL(triggered(QAction *)),this,SLOT(hide()));
}

void TrayMenu::showEvent(QShowEvent * event) {
    QMenu::showEvent(event);
    grabMouse();
}

bool TrayMenu::eventFilter(QObject * obj, QEvent *event) {
    bool ret = QMenu::eventFilter(obj,event);
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent * ev = (QMouseEvent *)event;
        if (ev->pos().x() < 0 || ev->pos().y() < 0) hide();
    }
    return ret;
}

void TrayMenu::leaveEvent(QEvent *) {
    grabMouse();
}

void TrayMenu::enterEvent(QEvent *) {
    releaseMouse();
}

MovieTrayIcon::MovieTrayIcon(QObject *parent) : QObject(parent) {
    first_time_menu_show = true;

    m_menu = new TrayMenu();

    connect(&m_tray_icon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(activatedReason(QSystemTrayIcon::ActivationReason)));
    connect(&m_tray_icon,SIGNAL(messageClicked()),this,SIGNAL(messageClicked()));
    connect(m_menu,SIGNAL(aboutToHide()),this,SLOT(realMenuHide()),Qt::DirectConnection);
    connect(&m_movie,SIGNAL(frameChanged(int)),this,SLOT(frameChanged(int)));
}

MovieTrayIcon::~MovieTrayIcon() {
    QMenu * menu = m_tray_icon.contextMenu();
    if (menu != NULL) delete menu;
    delete m_menu;
}

void MovieTrayIcon::realMenuHide() {
    TrayMenu * new_menu = new TrayMenu();
    QList<QAction *> actions = m_menu->actions();
    for (int i=0;i<actions.count();i++) {
        m_menu->removeAction(actions[i]);
        new_menu->addAction(actions[i]);
    }
    m_menu->deleteLater();
    m_menu = new_menu;
    connect(m_menu,SIGNAL(aboutToHide()),this,SLOT(realMenuHide()),Qt::DirectConnection);
}

void MovieTrayIcon::menuRequested() {
    emit aboutToShow();
    if (m_menu->actions().count() > 0) m_menu->exec(QCursor::pos());
}

void MovieTrayIcon::setVisible(bool flag) {
    if (flag) {
        if (first_time_menu_show) emit fillingMenuRequest(m_menu);
        first_time_menu_show = false;
        QMenu * menu = m_tray_icon.contextMenu();
        m_tray_icon.setContextMenu(new QMenu());
        connect(m_tray_icon.contextMenu(),SIGNAL(aboutToShow()),this,SLOT(menuRequested()));
        if (menu != NULL) menu->deleteLater();
    }
    m_tray_icon.setVisible(flag);
    if (flag && m_movie.frameCount() > 0) m_movie.start();
    else if (!flag) m_movie.stop();
}

void MovieTrayIcon::show() {
    setVisible(true);
}

void MovieTrayIcon::hide() {
    setVisible(false);
}

bool MovieTrayIcon::isVisible() const {
    return m_tray_icon.isVisible();
}

void MovieTrayIcon::setIcon(const QIcon & icon) {
    m_movie.stop();
    m_movie.clear();
    m_movie.setDelay(1);
    m_movie.add(icon);
    if (isVisible()) m_movie.start();
}

void MovieTrayIcon::setIcon(const QString & iconname) {
    m_movie.stop();
    m_movie.clear();

    if (QFileInfo(iconname).suffix().toLower() == "gif") {
        QMovie movie(iconname);
        m_movie.add(&movie);
        if (isVisible()) m_movie.start();
    }
    else {
        setIcon(QIcon(iconname));
    }
}

void MovieTrayIcon::setIcon(const QString & iconname,int input_frame_height,int delay) {
    m_movie.stop();
    m_movie.clear();
    int width = (100/input_frame_height)*input_frame_height;
    m_movie.setSize(QSize(width,width));
    m_movie.setDelay(delay);
    m_movie.add(iconname,input_frame_height);
    if (isVisible()) m_movie.start();

}

void MovieTrayIcon::frameChanged(int id) {
    m_tray_icon.setIcon(QIcon(m_movie.frame(id)));
}

void MovieTrayIcon::showMessage(const QString & title,const QString & msg,const QIcon &icon,int msecs) {
    m_tray_icon.showMessage(title,truncateMsg(msg).join('\n'),icon,msecs);
}

void MovieTrayIcon::showMessage(const QString & title,const QString & msg,QSystemTrayIcon::MessageIcon icon,int msecs) {
    m_tray_icon.showMessage(title,truncateMsg(msg).join('\n'),icon,msecs);
}

void MovieTrayIcon::setToolTip(const QString & title,const QString & msg) {
    m_tray_icon.setToolTip(title+"\n"+truncateMsg(msg).join('\n'));
}

void MovieTrayIcon::clearTooltip() {
    m_tray_icon.setToolTip(QString());
}

void MovieTrayIcon::activatedReason(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
        emit doubleClicked();
        break;
    case QSystemTrayIcon::Trigger:
        emit clicked();
        break;
    case QSystemTrayIcon::MiddleClick:
        emit middleClicked();
        break;
    default:
        break;
    }
}

QStringList MovieTrayIcon::truncateMsg(const QString & msg) const {
    QStringList ret;
    QStringList lines = msg.split("\n");
    int count = qMin(lines.count(),MAX_LINE_MSG_COUNT);
    for (int i=0;i<count;i++) {
        ret << lines[i];
    }
    if (lines.count() > MAX_LINE_MSG_COUNT) ret << QString("...");
    return ret;
}

QRect MovieTrayIcon::geometry() const {
    return m_tray_icon.geometry();
}
