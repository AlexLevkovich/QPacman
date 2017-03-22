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

INCLUDEPATH += $$PWD/../libqpacman
LIBS += -L$$OUT_PWD/../libqpacman -lqpacman
QMAKE_LFLAGS += "-Wl,-rpath,\'\$$OUT_PWD/../libqpacman\'"

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

OGG123_BIN = $$system(which ogg123 2>/dev/null)
isEmpty( OGG123_BIN ):error( "ogg123 should be installed!!!" )
DEFINES += OGG123_BIN=\\\"$$OGG123_BIN\\\"

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman

DEFINES += INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"
DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"

SOURCES += main.cpp\
        mainwindow.cpp \
    singleapplication.cpp \
    toolbarwindow.cpp \
    jsondbsignals.cpp \
    externalplayer.cpp

HEADERS  += mainwindow.h \
    singleapplication.h \
    toolbarwindow.h \
    jsondbsignals.h \
    externalplayer.h

FORMS    += mainwindow.ui \
            toolbarwindow.ui

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
