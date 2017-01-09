#-------------------------------------------------
#
# Project created by QtCreator 2013-12-13T08:18:24
#
#-------------------------------------------------

QT       += core gui network dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QPacmanTray
TEMPLATE = app
INCLUDEPATH += .

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman

DEFINES += INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"
DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"

SU_BIN = $$system(which su 2>/dev/null)
isEmpty( SU_BIN ):error( "su should be installed!!!" )
DEFINES += SU_BIN=\\\"$$SU_BIN\\\"

PACMAN_BIN = $$system(which pacman 2>/dev/null)
isEmpty( PACMAN_BIN ):error( "pacman should be installed!!!" )
DEFINES += PACMAN_BIN=\\\"$$PACMAN_BIN\\\"

WGET_BIN = $$system(which wget 2>/dev/null)
isEmpty( WGET_BIN ):error( "wget should be installed!!!" )
DEFINES += WGET_BIN=\\\"$$WGET_BIN\\\"

BASH_BIN = $$system(which bash 2>/dev/null)
isEmpty( BASH_BIN ):error( "bash should be installed!!!" )
DEFINES += BASH_BIN=\\\"$$BASH_BIN\\\"

RM_BIN = $$system(which rm 2>/dev/null)
isEmpty( RM_BIN ):error( "rm should be installed!!!" )
DEFINES += RM_BIN=\\\"$$RM_BIN\\\"

KILL_BIN = $$system(which kill 2>/dev/null)
isEmpty( KILL_BIN ):error( "kill should be installed!!!" )
DEFINES += KILL_BIN=\\\"$$KILL_BIN\\\"

PSTREE_BIN = $$system(which pstree 2>/dev/null)
isEmpty( PSTREE_BIN ):error( "pstree should be installed!!!" )
DEFINES += PSTREE_BIN=\\\"$$PSTREE_BIN\\\"

AWK_BIN = $$system(which awk 2>/dev/null)
isEmpty( AWK_BIN ):error( "awk should be installed!!!" )
DEFINES += AWK_BIN=\\\"$$AWK_BIN\\\"

TR_BIN = $$system(which tr 2>/dev/null)
isEmpty( TR_BIN ):error( "tr should be installed!!!" )
DEFINES += TR_BIN=\\\"$$TR_BIN\\\"

lessThan(QT_MAJOR_VERSION, 5): {
SOURCES += qlockfile.cpp \
           qlockfile_unix.cpp
HEADERS += qlockfile.h \
           qlockfile_p.h
}

SOURCES += main.cpp\
        mainwindow.cpp \
    singleapplication.cpp \
    pacmandbrefresher.cpp \
    pacmanprocessreader.cpp \
    errordialog.cpp \
    pacmansimpleupdatesreader.cpp \
    toolbarwindow.cpp \
    jsondbsignals.cpp \
    externalplayer.cpp \
    wraplabel.cpp \
    suchecker.cpp \
    confsettings.cpp \
    pacmansetupinforeader.cpp \
    rootdialog.cpp \
    simplecrypt.cpp

HEADERS  += mainwindow.h \
    singleapplication.h \
    pacmandbrefresher.h \
    pacmanprocessreader.h \
    errordialog.h \
    pacmansimpleupdatesreader.h \
    toolbarwindow.h \
    jsondbsignals.h \
    externalplayer.h \
    wraplabel.h \
    suchecker.h \
    confsettings.h \
    pacmansetupinforeader.h \
    rootdialog.h \
    simplecrypt.h

FORMS    += mainwindow.ui \
            toolbarwindow.ui \
    rootdialog.ui

RESOURCES += \
    pics.qrc \

TRANSLATIONS = $$PWD/translations/qpacmantray_ru.ts \
               $$PWD/translations/qpacmantray_be.ts

LUPDATE = $$[QT_INSTALL_BINS]/lupdate -locations relative -no-ui-lines -no-sort
LRELEASE = $$[QT_INSTALL_BINS]/lrelease

updatets.files = TRANSLATIONS
updatets.commands = $$LUPDATE $$PWD/QPacmanTray.pro

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

desktop.files = QPacmanTray.desktop
desktop.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/applications/

icon.files = pics/PacmanTray-arch_logo.png
icon.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/pixmaps/

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/

INSTALLS += target qm desktop icon
