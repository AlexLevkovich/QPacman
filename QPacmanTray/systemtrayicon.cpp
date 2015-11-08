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

#include "systemtrayicon.h"

#include <QMenu>
#include <QEvent>
#include <QPoint>
#include <QLabel>
#include <QPushButton>
#include <QPainterPath>
#include <QPainter>
#include <QStyle>
#include <QGridLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QBitmap>
#include "wraplabel.h"
#include "balloontip.h"

#include "systemtrayicon_p.h"

SystemTrayIcon::SystemTrayIcon(QObject *parent) : QObject(parent)
{
    d = new SystemTrayIconPrivate(this);
}

SystemTrayIcon::SystemTrayIcon(const QIcon &icon, QObject *parent) : QObject(parent)
{
    d = new SystemTrayIconPrivate(this);
    setIcon(icon);
}

SystemTrayIcon::~SystemTrayIcon()
{
    d->remove_sys();
    delete d;
}

void SystemTrayIcon::setContextMenu(QMenu *menu)
{
    d->menu = menu;
    d->updateMenu_sys();
}

QMenu* SystemTrayIcon::contextMenu() const
{
    return d->menu;
}

void SystemTrayIcon::setIcon(const QIcon &icon)
{
    d->icon = icon;
    d->updateIcon_sys();
}

QIcon SystemTrayIcon::icon() const
{
    return d->icon;
}

void SystemTrayIcon::setToolTip(const QString &tooltip)
{
    d->toolTip = tooltip;
    d->updateToolTip_sys();
}

QString SystemTrayIcon::toolTip() const
{
    return d->toolTip;
}

QRect SystemTrayIcon::geometry() const
{
    if (!d->visible)
        return QRect();
    return d->geometry_sys();
}

void SystemTrayIcon::setVisible(bool visible)
{
    if (visible == d->visible)
        return;
    if (d->icon.isNull() && visible)
        qWarning("SystemTrayIcon::setVisible: No Icon set");
    d->visible = visible;
    if (d->visible)
        d->install_sys();
    else
        d->remove_sys();
}

bool SystemTrayIcon::isVisible() const
{
    return d->visible;
}

bool SystemTrayIcon::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        return d->sys->deliverToolTipEvent(e);
    }
    return QObject::event(e);
}

bool SystemTrayIcon::isSystemTrayAvailable()
{
    return SystemTrayIconPrivate::isSystemTrayAvailable_sys();
}

bool SystemTrayIcon::supportsMessages()
{
    return SystemTrayIconPrivate::supportsMessages_sys();
}

void SystemTrayIcon::showMessage(const QString& title, const QString& msg,
                            SystemTrayIcon::MessageIcon icon, int msecs)
{
    if (d->visible)
        d->showMessage_sys(msg, title, icon, msecs);
}

void SystemTrayIcon::hideMessageBalloon() {
    return BalloonTip::hideBalloon();
}

//////////////////////////////////////////////////////////////////////

void qtsystray_sendActivated(SystemTrayIcon *i, int r)
{
    emit i->activated((SystemTrayIcon::ActivationReason)r);
}

