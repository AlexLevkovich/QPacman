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

#ifndef SystemTrayIcon_P_H
#define SystemTrayIcon_P_H

#include "systemtrayicon.h"

#include <QMenu>
#include <QList>
#include <QPixmap>
#include <QString>
#include <QPointer>

class SystemTrayIconSys;
class QToolButton;
class QLabel;

class SystemTrayIconPrivate
{

public:
    SystemTrayIconPrivate(SystemTrayIcon * _q) : sys(0), q(_q), visible(false) { }

    void install_sys();
    void remove_sys();
    void updateIcon_sys();
    void updateToolTip_sys();
    void updateMenu_sys();
    QRect geometry_sys() const;
    void showMessage_sys(const QString &msg, const QString &title, SystemTrayIcon::MessageIcon icon, int secs);

    static bool isSystemTrayAvailable_sys();
    static bool supportsMessages_sys();

    QPointer<QMenu> menu;
    QIcon icon;
    QString toolTip;
    SystemTrayIconSys *sys;
    bool visible;
    SystemTrayIcon * q;
};

#include <X11/Xmu/Atoms.h>

class SystemTrayIconSys : public QWidget
{
    friend class SystemTrayIconPrivate;
    friend class SystemTrayNativeEventFilter;

public:
    SystemTrayIconSys(SystemTrayIcon *q);
    ~SystemTrayIconSys();
    enum {
        SYSTEM_TRAY_REQUEST_DOCK = 0,
        SYSTEM_TRAY_BEGIN_MESSAGE = 1,
        SYSTEM_TRAY_CANCEL_MESSAGE =2
    };

    void addToTray();
    void updateIcon();
    XVisualInfo* getSysTrayVisualInfo();
    QRect globalGeometry() const;

    // QObject::event is public but QWidget's ::event() re-implementation
    // is protected ;(
    inline bool deliverToolTipEvent(QEvent *e)
    { return QWidget::event(e); }

    static Window sysTrayWindow;
    static QList<SystemTrayIconSys *> trayIcons;
#if QT_VERSION < 0x050000
    static QCoreApplication::EventFilter oldEventFilter;
    static bool sysTrayTracker(void *message, long *result);
#endif
    static Window locateSystemTray();
    static Atom sysTraySelection;
    static XVisualInfo sysTrayVisual;

protected:
    void paintEvent(QPaintEvent *pe);
    void resizeEvent(QResizeEvent *re);
#if QT_VERSION < 0x050000
    bool x11Event(XEvent *event);
#endif
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *event);
#endif
    bool event(QEvent *e);

private:
    QPixmap background;
    SystemTrayIcon *q;
    Colormap colormap;

    static bool eventFilterAdded;
};

#endif // SystemTrayIcon_P_H

