/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "movietrayicon.h"
#include <QMovie>
#include <QMenu>
#include <QCloseEvent>
#include <QApplication>
#include <QFileInfo>
#include <QTimer>
#include <QMouseEvent>
#include <QWindow>
#include <QDebug>

const int MovieTrayIcon::MAX_LINE_MSG_COUNT = 6;

MovieTrayIcon::MovieTrayIcon(QObject *parent) : QObject(parent) {
    first_time_menu_show = true;

    m_tray_icon = new QSystemTrayIcon();
    m_menu = new QMenu();

    connect(m_tray_icon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(activatedReason(QSystemTrayIcon::ActivationReason)));
    connect(m_tray_icon,SIGNAL(messageClicked()),this,SIGNAL(messageClicked()));
    connect(&m_movie,SIGNAL(frameChanged(int)),this,SLOT(frameChanged(int)));
    connect(m_menu,&QMenu::aboutToShow,[&]() { emit menuAboutToShow(m_menu); });
    qApp->installEventFilter(this);

    QMetaObject::invokeMethod(this,"setMenu",Qt::QueuedConnection);
}

MovieTrayIcon::~MovieTrayIcon() {
    if (m_menu != NULL) delete m_menu;
    m_menu = NULL;
    if (m_tray_icon != NULL) delete m_tray_icon;
    m_tray_icon = NULL;
}

bool MovieTrayIcon::eventFilter(QObject *watched, QEvent *event) {
    bool ret = QObject::eventFilter(watched,event);
    if (watched == this || (event->type() != QEvent::Close)) return ret;

    QMetaObject::invokeMethod(this,"checkTopLevelWindows",Qt::QueuedConnection);

    return ret;
}

void MovieTrayIcon::checkTopLevelWindows() {
    if (m_menu == NULL) return;
    if (m_tray_icon == NULL) return;
    bool is_another = false;
    for (QWindow * wnd: QGuiApplication::topLevelWindows()) {
        if (wnd != m_menu->windowHandle() && (wnd->visibility() != QWindow::Hidden)) {
            is_another = true;
            break;
        }
    }
    if (!is_another && QGuiApplication::quitOnLastWindowClosed()) {
        QSystemTrayIcon * tray = m_tray_icon;
        m_tray_icon = NULL;
        delete tray;
        QMenu * menu = m_menu;
        m_menu = NULL;
        delete menu;
    }
}

void MovieTrayIcon::setMenu() {
    if (m_tray_icon == NULL) return;
    if (m_menu == NULL) return;
    initMenu(m_menu);
    m_tray_icon->setContextMenu(m_menu);
}

void MovieTrayIcon::initMenu(QMenu *) {}

void MovieTrayIcon::setVisible(bool flag) {
    QMetaObject::invokeMethod(this,"setTrayVisible",Qt::QueuedConnection,Q_ARG(bool,flag));
}

void MovieTrayIcon::setTrayVisible(bool flag) {
    if (m_tray_icon == NULL) return;
    m_tray_icon->setVisible(flag);
    if (flag && m_movie.frameCount() > 0) m_movie.start();
    else if (!flag) m_movie.stop();
}

void MovieTrayIcon::show() {
    QMetaObject::invokeMethod(this,"setTrayVisible",Qt::QueuedConnection,Q_ARG(bool,true));
}

void MovieTrayIcon::hide() {
    QMetaObject::invokeMethod(this,"setTrayVisible",Qt::QueuedConnection,Q_ARG(bool,false));
}

bool MovieTrayIcon::isVisible() const {
    if (m_tray_icon == NULL) return false;
    return m_tray_icon->isVisible();
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
   if (m_tray_icon == NULL) return;
    m_tray_icon->setIcon(QIcon(m_movie.frame(id)));
}

void MovieTrayIcon::showMessage(const QString & title,const QString & msg,const QIcon &icon,int msecs) {
    if (m_tray_icon == NULL) return;
    m_tray_icon->showMessage(title,truncateMsg(msg).join('\n'),icon,msecs);
}

void MovieTrayIcon::showMessage(const QString & title,const QString & msg,QSystemTrayIcon::MessageIcon icon,int msecs) {
    if (m_tray_icon == NULL) return;
    m_tray_icon->showMessage(title,truncateMsg(msg).join('\n'),icon,msecs);
}

void MovieTrayIcon::setToolTip(const QString & title,const QString & msg) {
    if (m_tray_icon == NULL) return;
    m_tray_icon->setToolTip(title+"\n"+truncateMsg(msg).join('\n'));
}

void MovieTrayIcon::clearTooltip() {
    if (m_tray_icon == NULL) return;
    m_tray_icon->setToolTip(QString());
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
    if (m_tray_icon == NULL) return QRect();
    return m_tray_icon->geometry();
}
