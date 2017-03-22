QT += core
QT -= gui

TARGET = qpacmankill
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

unix:CONFIG += link_pkgconfig
unix:PKGCONFIG += libprocps

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

PACMAN_BIN = $$system(which pacman 2>/dev/null)
isEmpty( PACMAN_BIN ):error( "pacman should be installed!!!" )
DEFINES += PACMAN_BIN=\\\"$$PACMAN_BIN\\\"

DEFINES += QT_DEPRECATED_WARNINGS

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
INSTALLS += target

unset(MANGLED_INSTALLS)
for(x, INSTALLS):MANGLED_INSTALLS += install_$${x}

suidqpacmankill.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
suidqpacmankill.commands = chown root:root $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/qpacmankill; chmod 4755 $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/qpacmankill
suidqpacmankill.depends = $${MANGLED_INSTALLS}
INSTALLS += suidqpacmankill

