QT += core
QT -= gui

CONFIG += c++11

TARGET = pacmanrmcache
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

DEFINES += QT_DEPRECATED_WARNINGS

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

PACMAN_BIN = $$system(which pacman 2>/dev/null)
isEmpty( PACMAN_BIN ):error( "pacman should be installed!!!" )
DEFINES += PACMAN_BIN=\\\"$$PACMAN_BIN\\\"

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
INSTALLS += target

unset(MANGLED_INSTALLS)
for(x, INSTALLS):MANGLED_INSTALLS += install_$${x}

suidpacmanrmcache.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
suidpacmanrmcache.commands = chown root:root $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/pacmanrmcache; chmod 4755 $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/pacmanrmcache
suidpacmanrmcache.depends = $${MANGLED_INSTALLS}
INSTALLS += suidpacmanrmcache

