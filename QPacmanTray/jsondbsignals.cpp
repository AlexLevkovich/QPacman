/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtAddOn.JsonDb module of the Qt Toolkit.
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

/********************************************************************************
** Modified by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <QSocketNotifier>
#include "jsondbsignals.h"

int JsonDbSignals::sSigFD[2];

JsonDbSignals::JsonDbSignals( QObject *parent ) : QObject(parent) {
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sSigFD))
        qFatal("Unable to create signal socket pair");

    mNotifier = new QSocketNotifier(sSigFD[1], QSocketNotifier::Read, this);
    connect(mNotifier, SIGNAL(activated(int)), this, SLOT(handleSig()));
}

void JsonDbSignals::start() {
    if (receivers(SIGNAL(sigTERM())) > 0)
        listen(SIGTERM);

    if (receivers(SIGNAL(sigHUP())) > 0)
        listen(SIGHUP);

    if (receivers(SIGNAL(sigINT())) > 0)
        listen(SIGINT);

    if (receivers(SIGNAL(sigILL())) > 0)
        listen(SIGILL);

    if (receivers(SIGNAL(sigBUS())) > 0)
        listen(SIGBUS);

    if (receivers(SIGNAL(sigQUIT())) > 0)
        listen(SIGQUIT);

    if (receivers(SIGNAL(sigSEGV())) > 0)
        listen(SIGSEGV);
}

void JsonDbSignals::signalHandler(int number) {
    int tmp = number;
    ::write(sSigFD[0], &tmp, sizeof(tmp));
}

void JsonDbSignals::handleSig() {
    mNotifier->setEnabled(false);
    int tmp;
    ::read(sSigFD[1], &tmp, sizeof(tmp));
    switch (tmp) {
    case SIGTERM:
        emit sigTERM();
        break;
    case SIGHUP:
        emit sigHUP();
        break;
    case SIGINT:
        emit sigINT();
        break;
    case SIGILL:
        emit sigILL();
        break;
    case SIGBUS:
        emit sigBUS();
        break;
    case SIGQUIT:
        emit sigQUIT();
        break;
    case SIGSEGV:
        emit sigSEGV();
        break;
    default:
        break;
    }
    mNotifier->setEnabled(true);
}

void JsonDbSignals::listen(int sig) {
    struct sigaction action;
    action.sa_handler = JsonDbSignals::signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_flags |= SA_RESTART;

    if (::sigaction(sig, &action, 0) < 0)
        qFatal("Unable to set sigaction on signal %d", sig);
}
