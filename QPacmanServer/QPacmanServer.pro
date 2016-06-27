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

TOOLS_BIN = /usr/bin

!exists( $$TOOLS_BIN/pacman ) {
    error( "pacman should be installed!!!" )
}

!exists( $$TOOLS_BIN/tar ) {
    error( "tar should be installed!!!" )
}

!exists( $$TOOLS_BIN/xz ) {
    error( "xz should be installed!!!" )
}

!exists( $$TOOLS_BIN/stdbuf ) {
    error( "coreutils should be installed!!!" )
}

!exists( $$TOOLS_BIN/wget ) {
    error( "wget should be installed!!!" )
}

!exists( $$TOOLS_BIN/ps ) {
    error( "procps-ng should be installed!!!" )
}

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman

DEFINES += INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"
DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"
DEFINES += TOOLS_BIN=\\\"$$TOOLS_BIN\\\"

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
