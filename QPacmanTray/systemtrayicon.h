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

#ifndef SystemTrayIcon_H
#define SystemTrayIcon_H

#include <QObject>
#include <QIcon>
#include <QUrl>

class QMenu;
class QEvent;
class QWheelEvent;
class QMouseEvent;
class QPoint;

class SystemTrayIconPrivate;

class SystemTrayIcon : public QObject
{
    Q_OBJECT

public:
    SystemTrayIcon(QObject *parent = 0);
    SystemTrayIcon(const QIcon &icon, QObject *parent = 0);
    ~SystemTrayIcon();

    enum ActivationReason {
        Unknown,
        Context,
        DoubleClick,
        Trigger,
        MiddleClick,
        ToolTip
    };

    void setContextMenu(QMenu *menu);
    QMenu *contextMenu() const;

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    QString toolTip() const;
    void setToolTip(const QString &tip);

    static bool isSystemTrayAvailable();
    static bool supportsMessages();

    enum MessageIcon { NoIcon, Information, Warning, Critical };
    void showMessage(const QString &title, const QString &msg,
                     MessageIcon icon = Information, int msecs = 10000);
    void hideMessageBalloon();

    QRect geometry() const;
    bool isVisible() const;

public Q_SLOTS:
    void setVisible(bool visible);
    inline void show() { setVisible(true); }
    inline void hide() { setVisible(false); }

Q_SIGNALS:
    void activated(SystemTrayIcon::ActivationReason reason);
    void messageClicked();
    void anchorClicked(const QUrl & link);
    void messageWindowShowUp();
    void messageWindowDestroyed();

protected:
    bool event(QEvent *event);

private:
    SystemTrayIconPrivate * d;

    friend class SystemTrayIconSys;
    friend class BalloonTip;
    friend void qtsystray_sendActivated(SystemTrayIcon *, int);
};

#endif // SystemTrayIcon_H
