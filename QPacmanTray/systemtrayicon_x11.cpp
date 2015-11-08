/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QMouseEvent>
#include <QApplication>
#include <QList>
#include <QMenu>
#include <QTimer>
#include <QX11Info>
#include <QPaintEngine>
#if QT_VERSION >= 0x050000
#include <QAbstractNativeEventFilter>
#endif
#include <QDebug>
#include <balloontip.h>
#include "systemtrayicon_p.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <xcb/xcb.h>

Window SystemTrayIconSys::sysTrayWindow = None;
QList<SystemTrayIconSys *> SystemTrayIconSys::trayIcons;
#if QT_VERSION < 0x050000
QCoreApplication::EventFilter SystemTrayIconSys::oldEventFilter = 0;
#endif
Atom SystemTrayIconSys::sysTraySelection = None;
XVisualInfo SystemTrayIconSys::sysTrayVisual = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool SystemTrayIconSys::eventFilterAdded = false;

// Locate the system tray
Window SystemTrayIconSys::locateSystemTray()
{
    Display *display = QX11Info::display();
    if (sysTraySelection == None) {
        int screen = QX11Info::appScreen();
        QString net_sys_tray = QString::fromLatin1("_NET_SYSTEM_TRAY_S%1").arg(screen);
        sysTraySelection = XInternAtom(display, net_sys_tray.toLatin1(), False);
    }

    return XGetSelectionOwner(QX11Info::display(), sysTraySelection);
}

QRect SystemTrayIconSys::globalGeometry() const
{
    ::Window dummy;
    int x, y, rootX, rootY;
    unsigned int width, height, border, depth;
    // Use X11 API since we are parented on the tray, about which the QWindow does not know.
    XGetGeometry(QX11Info::display(), winId(), &dummy, &x, &y, &width, &height, &border, &depth);
    XTranslateCoordinates(QX11Info::display(), winId(),
                          XRootWindow(QX11Info::display(),QX11Info::appScreen()),
                          x, y, &rootX, &rootY, &dummy);
    return QRect(QPoint(rootX, rootY), QSize(width, height));
}

XVisualInfo* SystemTrayIconSys::getSysTrayVisualInfo()
{
    Display *display = QX11Info::display();

    if (!sysTrayVisual.visual) {
        Window win = locateSystemTray();
        if (win != None) {
            Atom actual_type;
            int actual_format;
            ulong nitems, bytes_remaining;
            uchar *data = 0;
            int result = XGetWindowProperty(display, win, XInternAtom(display,"_NET_SYSTEM_TRAY_VISUAL",False), 0, 1,
                                            False, XA_VISUALID, &actual_type,
                                            &actual_format, &nitems, &bytes_remaining, &data);
            VisualID vid = 0;
            if (result == Success && data && actual_type == XA_VISUALID && actual_format == 32 &&
                nitems == 1 && bytes_remaining == 0)
                vid = *(VisualID*)data;
            if (data)
                XFree(data);
            if (vid == 0)
                return 0;

            uint mask = VisualIDMask;
            XVisualInfo *vi, rvi;
            int count;
            rvi.visualid = vid;
            vi = XGetVisualInfo(display, mask, &rvi, &count);
            if (vi) {
                sysTrayVisual = vi[0];
                XFree((char*)vi);
            }
            if (sysTrayVisual.depth != 32)
                memset(&sysTrayVisual, 0, sizeof(sysTrayVisual));
        }
    }

    return sysTrayVisual.visual ? &sysTrayVisual : 0;
}

#if QT_VERSION >= 0x050000
class SystemTrayNativeEventFilter : public QAbstractNativeEventFilter {
public:
    SystemTrayNativeEventFilter() {}
    bool nativeEventFilter(const QByteArray & eventType,void * message,long *) {
        if (eventType == "xcb_generic_event_t") {
            xcb_generic_event_t * event = (xcb_generic_event_t *)message;
            uint8_t xcb_event_type = event->response_type & ~0x80;
            if (xcb_event_type == XCB_REPARENT_NOTIFY) {
                xcb_reparent_notify_event_t  * reparent_event = (xcb_reparent_notify_event_t  *)event;
                for (int i = 0; i < SystemTrayIconSys::trayIcons.count(); i++) {
                    if (reparent_event->window == SystemTrayIconSys::trayIcons[i]->winId()) {
                        SystemTrayIconSys::trayIcons[i]->show();
                        break;
                    }
                }
            }
            else {
                bool retval = false;

                if (SystemTrayIconSys::trayIcons.isEmpty())
                    return retval;

                Display *display = QX11Info::display();
                if  (xcb_event_type == XCB_DESTROY_NOTIFY && ((xcb_destroy_notify_event_t *)event)->window == SystemTrayIconSys::sysTrayWindow) {
                    SystemTrayIconSys::sysTrayWindow = SystemTrayIconSys::locateSystemTray();
                    memset(&SystemTrayIconSys::sysTrayVisual, 0, sizeof(SystemTrayIconSys::sysTrayVisual));
                    for (int i = 0; i < SystemTrayIconSys::trayIcons.count(); i++) {
                        if (SystemTrayIconSys::sysTrayWindow == None) {
                        BalloonTip::hideBalloon();
                            SystemTrayIconSys::trayIcons[i]->hide(); // still no luck
                            SystemTrayIconSys::trayIcons[i]->destroy();
                            SystemTrayIconSys::trayIcons[i]->create();
                    } else
                            SystemTrayIconSys::trayIcons[i]->addToTray(); // add it to the new tray
                    }
                    retval = true;
                } else if (xcb_event_type == XCB_CLIENT_MESSAGE && SystemTrayIconSys::sysTrayWindow == None) {
                    static Atom manager_atom = XInternAtom(display, "MANAGER", False);
                    XClientMessageEvent *cm = (XClientMessageEvent *)message;
                    if ((cm->message_type == manager_atom) && ((Atom)cm->data.l[1] == SystemTrayIconSys::sysTraySelection)) {
                    SystemTrayIconSys::sysTrayWindow = cm->data.l[2];
                        memset(&SystemTrayIconSys::sysTrayVisual, 0, sizeof(SystemTrayIconSys::sysTrayVisual));
                    XSelectInput(display, SystemTrayIconSys::sysTrayWindow, StructureNotifyMask);
                        for (int i = 0; i < SystemTrayIconSys::trayIcons.count(); i++) {
                            SystemTrayIconSys::trayIcons[i]->addToTray();
                        }
                        retval = true;
                    }
                } else if (xcb_event_type == XCB_PROPERTY_NOTIFY && ((xcb_property_notify_event_t *)event)->atom == XInternAtom(display,"_NET_SYSTEM_TRAY_VISUAL",False) &&
                           ((xcb_property_notify_event_t *)event)->window == SystemTrayIconSys::sysTrayWindow) {
                    memset(&SystemTrayIconSys::sysTrayVisual, 0, sizeof(SystemTrayIconSys::sysTrayVisual));
                    for (int i = 0; i < SystemTrayIconSys::trayIcons.count(); i++) {
                        SystemTrayIconSys::trayIcons[i]->addToTray();
                    }
                }

                return retval;
            }
        }
        return false;
    }
};
#endif

#if QT_VERSION < 0x050000
bool SystemTrayIconSys::sysTrayTracker(void *message, long *result)
{
    bool retval = false;
    if (SystemTrayIconSys::oldEventFilter)
        retval = SystemTrayIconSys::oldEventFilter(message, result);

    if (trayIcons.isEmpty())
        return retval;

    Display *display = QX11Info::display();
    XEvent *ev = (XEvent *)message;
    if  (ev->type == DestroyNotify && ev->xany.window == sysTrayWindow) {
	sysTrayWindow = locateSystemTray();
        memset(&sysTrayVisual, 0, sizeof(sysTrayVisual));
        for (int i = 0; i < trayIcons.count(); i++) {
            if (sysTrayWindow == None) {
	        BalloonTip::hideBalloon();
                trayIcons[i]->hide(); // still no luck
                trayIcons[i]->destroy();
                trayIcons[i]->create();
	    } else
                trayIcons[i]->addToTray(); // add it to the new tray
        }
        retval = true;
    } else if (ev->type == ClientMessage && sysTrayWindow == None) {
        static Atom manager_atom = XInternAtom(display, "MANAGER", False);
        XClientMessageEvent *cm = (XClientMessageEvent *)message;
        if ((cm->message_type == manager_atom) && ((Atom)cm->data.l[1] == sysTraySelection)) {
	    sysTrayWindow = cm->data.l[2];
            memset(&sysTrayVisual, 0, sizeof(sysTrayVisual));
	    XSelectInput(display, sysTrayWindow, StructureNotifyMask);
            for (int i = 0; i < trayIcons.count(); i++) {
                trayIcons[i]->addToTray();
            }
            retval = true;
        }
    } else if (ev->type == PropertyNotify && ev->xproperty.atom == XInternAtom(display,"_NET_SYSTEM_TRAY_VISUAL",False) &&
               ev->xproperty.window == sysTrayWindow) {
        memset(&sysTrayVisual, 0, sizeof(sysTrayVisual));
        for (int i = 0; i < trayIcons.count(); i++) {
            trayIcons[i]->addToTray();
        }
    }

    return retval;
}
#endif

SystemTrayIconSys::SystemTrayIconSys(SystemTrayIcon *q)
    : QWidget(0, Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint),
      q(q), colormap(0)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    //setAttribute(Qt::WA_PaintOnScreen);  may be for qt4 should be on

    Display *display = QX11Info::display();
    if (!eventFilterAdded) {
#if QT_VERSION < 0x050000
        oldEventFilter = qApp->setEventFilter(sysTrayTracker);
#else
        qApp->installNativeEventFilter(new SystemTrayNativeEventFilter());
#endif
	eventFilterAdded = true;
	Window root = QX11Info::appRootWindow();
        XWindowAttributes attr;
        XGetWindowAttributes(display, root, &attr);
        if ((attr.your_event_mask & StructureNotifyMask) != StructureNotifyMask) {
            (void) QApplication::desktop(); // lame trick to ensure our event mask is not overridden
            XSelectInput(display, root, attr.your_event_mask | StructureNotifyMask); // for MANAGER selection
        }
    }

    if (trayIcons.isEmpty()) {
        sysTrayWindow = locateSystemTray();
        if (sysTrayWindow != None) XSelectInput(display, sysTrayWindow, StructureNotifyMask); // track tray events
    }
    trayIcons.append(this);
    setMouseTracking(true);
#ifndef QT_NO_TOOLTIP
    setToolTip(q->toolTip());
#endif
    if (sysTrayWindow != None)
        addToTray();
}

SystemTrayIconSys::~SystemTrayIconSys()
{
    trayIcons.removeAt(trayIcons.indexOf(this));
    Display *display = QX11Info::display();
    if (trayIcons.isEmpty()) {
        if (sysTrayWindow == None)
            return;
        if (display)
            XSelectInput(display, sysTrayWindow, 0); // stop tracking the tray
        sysTrayWindow = None;
    }
    if (colormap)
        XFreeColormap(display, colormap);
}

void SystemTrayIconSys::addToTray()
{
    Q_ASSERT(sysTrayWindow != None);
    Display *display = QX11Info::display();

    XVisualInfo *vi = getSysTrayVisualInfo();
    if (vi && vi->visual) {
        Window root = RootWindow(display, vi->screen);
        Window p = root;
        if (QWidget *pw = parentWidget())
            p = pw->effectiveWinId();
        colormap = XCreateColormap(display, root, vi->visual, AllocNone);
        XSetWindowAttributes wsa;
        wsa.background_pixmap = 0;
        wsa.colormap = colormap;
        wsa.background_pixel = 0;
        wsa.border_pixel = 0;
        Window wid = XCreateWindow(display, p, -1, -1, 1, 1,
                                   0, vi->depth, InputOutput, vi->visual,
                                   CWBackPixmap|CWBackPixel|CWBorderPixel|CWColormap, &wsa);
        create(wid);
    } else {
        XSetWindowBackgroundPixmap(display, winId(), ParentRelative);
    }

    // GNOME, NET WM Specification
    static Atom netwm_tray_atom = XInternAtom(display, "_NET_SYSTEM_TRAY_OPCODE", False);
    long l[5] = { CurrentTime, SYSTEM_TRAY_REQUEST_DOCK, static_cast<long>(winId()), 0, 0 };
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = sysTrayWindow;
    ev.xclient.message_type = netwm_tray_atom;
    ev.xclient.format = 32;
    memcpy((char *)&ev.xclient.data, (const char *) l, sizeof(l));
    XSendEvent(display, sysTrayWindow, False, 0, &ev);
    setMinimumSize(22, 22); // required at least on GNOME
}

void SystemTrayIconSys::updateIcon()
{
    update();
}

void SystemTrayIconSys::resizeEvent(QResizeEvent *re)
{
     QWidget::resizeEvent(re);
     updateIcon();
}

void SystemTrayIconSys::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    if (!getSysTrayVisualInfo()) {
        const QRegion oldSystemClip = p.paintEngine()->systemClip();
        const QRect clearedRect = oldSystemClip.boundingRect();
        XClearArea(QX11Info::display(), winId(), clearedRect.x(), clearedRect.y(),
                   clearedRect.width(), clearedRect.height(), False);
        QPaintEngine *pe = p.paintEngine();
        pe->setSystemClip(clearedRect);
        q->icon().paint(&p, rect());
        pe->setSystemClip(oldSystemClip);
    } else {
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        q->icon().paint(&p, rect());
    }
}

void SystemTrayIconSys::mousePressEvent(QMouseEvent *ev)
{
    QPoint globalPos = ev->globalPos();
    if (ev->button() == Qt::RightButton && q->contextMenu())
        q->contextMenu()->popup(globalPos);

    if (BalloonTip::isBalloonVisible()) {
        emit q->messageClicked();
        BalloonTip::hideBalloon();
    }

    if (ev->button() == Qt::LeftButton)
        emit q->activated(SystemTrayIcon::Trigger);
    else if (ev->button() == Qt::RightButton)
        emit q->activated(SystemTrayIcon::Context);
    else if (ev->button() == Qt::MidButton)
        emit q->activated(SystemTrayIcon::MiddleClick);
}

void SystemTrayIconSys::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        emit q->activated(SystemTrayIcon::DoubleClick);
}

#ifndef QT_NO_WHEELEVENT
void SystemTrayIconSys::wheelEvent(QWheelEvent *e)
{
    QApplication::sendEvent(q, e);
}
#endif

bool SystemTrayIconSys::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        emit q->activated(SystemTrayIcon::ToolTip);
        return QApplication::sendEvent(q, e);
    }
    return QWidget::event(e);
}

#if QT_VERSION < 0x050000
bool SystemTrayIconSys::x11Event(XEvent *event)
{
    if (event->type == ReparentNotify)
        show();
    return QWidget::x11Event(event);
}
#endif

////////////////////////////////////////////////////////////////////////////
void SystemTrayIconPrivate::install_sys()
{
    if (!sys)
        sys = new SystemTrayIconSys(q);
}

QRect SystemTrayIconPrivate::geometry_sys() const
{
    if (!sys) return QRect();
    return QRect(sys->mapToGlobal(QPoint(0, 0)), sys->size());
}

void SystemTrayIconPrivate::remove_sys()
{
    if (!sys)
        return;
    BalloonTip::hideBalloon();
    sys->hide(); // this should do the trick, but...
    delete sys; // wm may resize system tray only for DestroyEvents
    sys = 0;
}

void SystemTrayIconPrivate::updateIcon_sys()
{
    if (!sys)
        return;
    sys->updateIcon();
}

void SystemTrayIconPrivate::updateMenu_sys()
{

}

void SystemTrayIconPrivate::updateToolTip_sys()
{
    if (!sys)
        return;
#ifndef QT_NO_TOOLTIP
    sys->setToolTip(toolTip);
#endif
}

bool SystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
    return SystemTrayIconSys::locateSystemTray() != None;
}

bool SystemTrayIconPrivate::supportsMessages_sys()
{
    return true;
}

void SystemTrayIconPrivate::showMessage_sys(const QString &message, const QString &title,
                                   SystemTrayIcon::MessageIcon icon, int msecs)
{
    if (!sys) return;
    //QPoint g = sys->mapToGlobal(QPoint(0, 0));
    QPoint g = sys->globalGeometry().topLeft();
    BalloonTip * tip = BalloonTip::showBalloon(icon, title, message, q,
                       QPoint(g.x() + sys->width()/2, g.y() + sys->height()/2),msecs,true);
    if (tip != NULL) QObject::connect(tip,SIGNAL(anchorClicked(const QUrl &)),q,SIGNAL(anchorClicked(const QUrl &)),Qt::QueuedConnection);
}

