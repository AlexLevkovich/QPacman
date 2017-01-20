TARGET = pacmanSy
TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.c

PACMAN_BIN = $$system(which pacman 2>/dev/null)
isEmpty( PACMAN_BIN ):error( "pacman should be installed!!!" )
DEFINES += PACMAN_BIN=\\\"$$PACMAN_BIN\\\"

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
INSTALLS += target

unset(MANGLED_INSTALLS)
for(x, INSTALLS):MANGLED_INSTALLS += install_$${x}

suidpacmanSy.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
suidpacmanSy.commands = chown root:root $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/pacmanSy; chmod 4755 $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/pacmanSy
suidpacmanSy.depends = $${MANGLED_INSTALLS}
INSTALLS += suidpacmanSy
