#-------------------------------------------------
#
# Project created by QtCreator 2013-12-07T15:10:05
#
#-------------------------------------------------

QT       += core network dbus
QT       -= gui

TARGET = QPacmanServer
CONFIG   += console
CONFIG   -= app_bundle

DEFINES  += IS_QPACMAN_SERVER
INCLUDEPATH += .

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr/local
}

PACMAN_BIN = $$system(which pacman 2>/dev/null)
isEmpty( PACMAN_BIN ):error( "pacman should be installed!!!" )
DEFINES += PACMAN_BIN=\\\"$$PACMAN_BIN\\\"

TAR_BIN = $$system(which tar 2>/dev/null)
isEmpty( TAR_BIN ):error( "tar should be installed!!!" )
DEFINES += TAR_BIN=\\\"$$TAR_BIN\\\"

XZ_BIN = $$system(which xz 2>/dev/null)
isEmpty( XZ_BIN ):error( "xz should be installed!!!" )

STDBUF_BIN = $$system(which stdbuf 2>/dev/null)
isEmpty( STDBUF_BIN ):error( "stdbuf should be installed!!!" )
DEFINES += STDBUF_BIN=\\\"$$STDBUF_BIN\\\"

WGET_BIN = $$system(which wget 2>/dev/null)
isEmpty( WGET_BIN ):error( "wget should be installed!!!" )
DEFINES += WGET_BIN=\\\"$$WGET_BIN\\\"

PS_BIN = $$system(which ps 2>/dev/null)
isEmpty( PS_BIN ):error( "ps should be installed!!!" )
DEFINES += PS_BIN=\\\"$$PS_BIN\\\"

CAT_BIN = $$system(which cat 2>/dev/null)
isEmpty( CAT_BIN ):error( "cat should be installed!!!" )
DEFINES += CAT_BIN=\\\"$$CAT_BIN\\\"

BASH_BIN = $$system(which bash 2>/dev/null)
isEmpty( BASH_BIN ):error( "bash should be installed!!!" )
DEFINES += BASH_BIN=\\\"$$BASH_BIN\\\"

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman

DEFINES += INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"
DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"

TEMPLATE = app

SOURCES += main.cpp \
    pacmanentry.cpp \
    pacmanprocessreader.cpp \
    pacmanrepositoryreader.cpp \
    pacmanfileslistreader.cpp \
    pacmansetupinforeader.cpp \
    pacmanfilepackageinforeader.cpp \
    pacmaninstalllocalpackagesreader.cpp \
    pacmaninstallpackagesreader.cpp \
    pacmanpackagereasonchanger.cpp \
    pacmanremovepackagesreader.cpp \
    singleapplication.cpp \
    pacmansimpleupdatesreader.cpp \
    byteshumanizer.cpp \
    jsondbsignals.cpp \
    pacmandbusserver.cpp \
    qlockfile.cpp \
    qlockfile_unix.cpp \
    pacmandbrefresher.cpp \
    confsettings.cpp

HEADERS += \
    pacmanentry.h \
    pacmanprocessreader.h \
    pacmanrepositoryreader.h \
    pacmanfileslistreader.h \
    pacmansetupinforeader.h \
    pacmanfilepackageinforeader.h \
    pacmaninstalllocalpackagesreader.h \
    pacmaninstallpackagesreader.h \
    pacmanpackagereasonchanger.h \
    pacmanremovepackagesreader.h \
    singleapplication.h \
    pacmansimpleupdatesreader.h \
    byteshumanizer.h \
    jsondbsignals.h \
    pacmandbusserver.h \
    qlockfile.h \
    qlockfile_p.h \
    pacmandbrefresher.h \
    confsettings.h

TRANSLATIONS = $$PWD/translations/qpacmanservice_ru.ts \
               $$PWD/translations/qpacmanservice_be.ts

LUPDATE = $$[QT_INSTALL_BINS]/lupdate -locations relative -no-ui-lines -no-sort
LRELEASE = $$[QT_INSTALL_BINS]/lrelease

updatets.files = TRANSLATIONS
updatets.commands = $$LUPDATE $$PWD/QPacmanServer.pro

QMAKE_EXTRA_TARGETS += updatets

updateqm.depends = updatets
updateqm.input = TRANSLATIONS
updateqm.output = translations/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.name = LRELEASE ${QMAKE_FILE_IN}
updateqm.variable_out = PRE_TARGETDEPS
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm

qm.files = $$TRANS_DIR1/*.qm
qm.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/qpacman/
qm.CONFIG += no_check_exist

service.files = $$PWD/xml/org.alexl.PacmanServer.service
service.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/dbus-1/system-services/

systemd.files = $$PWD/xml/QPacmanServer.service
systemd.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/systemd/system/
#systemd.extra = mkdir -p $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/systemd/system/dbus.target.wants/; ln -sf /usr/lib/systemd/system/QPacmanServer.service $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/systemd/system/dbus.target.wants/QPacmanServer.service

etc.files = $$PWD/xml/org.alexl.PacmanServer.conf
etc.path = $$INSTALL_ROOT/etc/dbus-1/system.d/

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/

INSTALLS += service systemd etc target qm
