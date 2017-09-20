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

LRELEASE = $$[QT_INSTALL_BINS]/lrelease
for(tsfile, TRANSLATIONS) {
    qmfile = $$basename(tsfile)
    qmfile ~= s,.ts$,.qm,
    qmdir = $$OUT_PWD/translations
    qmfile = $$qmdir/$$qmfile
    !exists($$qmdir) {
        system($${QMAKE_MKDIR} \"$$qmdir\")
    }
    command = $$LRELEASE $$tsfile -qm $$qmfile
    system($$command) | error("Failed to run: $$command")
}

LUPDATE = $$[QT_INSTALL_BINS]/lupdate -locations relative -no-ui-lines -no-sort
updatets.files = TRANSLATIONS
updatets.commands = $$LUPDATE $$PWD/QPacman.pro
QMAKE_EXTRA_TARGETS += updatets

transinstall.files = $$TRANS_DIR1/*.qm
transinstall.path = $$INSTALL_ROOT/$$TRANS_DIR2

desktop.files = QPacmanTray.desktop
desktop.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/applications/

icon.files = pics/PacmanTray-arch_logo.png
icon.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/pixmaps/

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/

INSTALLS += target transinstall desktop icon
