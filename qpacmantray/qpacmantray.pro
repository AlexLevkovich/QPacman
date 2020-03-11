#-------------------------------------------------
#
# Project created by QtCreator 2019-03-04T12:43:45
#
#-------------------------------------------------

QT       += multimedia widgets

TARGET = qpacmantray
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman
TRANS_DIR3 = $$OUT_PWD/../libs/qpacman/translations
TRANS_DIR4 = $$OUT_PWD/../libs/qalpm/translations

DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"
DEFINES += TRANS_DIR3=\\\"$$TRANS_DIR3\\\"
DEFINES += TRANS_DIR4=\\\"$$TRANS_DIR4\\\"

isEmpty(PACMANCONF) {
    PACMANCONF = /etc/pacman.conf
}
DEFINES += PACMANCONF=\\\"$$PACMANCONF\\\"

SOURCES += \
    alpmoptionswidget.cpp \
        main.cpp \
    qpacmantrayapplication.cpp \
    trayoptionswidget.cpp \
    traypreferences.cpp \
    qpacmantrayicon.cpp

HEADERS += \
    alpmoptionswidget.h \
    trayoptionswidget.h \
    traypreferences.h \
    qpacmantrayapplication.h \
    qpacmantrayicon.h

FORMS += \
    alpmoptionswidget.ui \
    trayoptionswidget.ui \
    traypreferences.ui

LANGUAGES = ru be

defineReplace(prependAll) {
 for(a,$$1):result += $${2}_$${a}$$3
 return($$result)
}

TRANSLATIONS = $$prependAll(LANGUAGES, $$PWD/translations/$$TARGET, .ts)

LRELEASE_COMMAND =
LRELEASE_TARGET =
LRELEASE = $$[QT_INSTALL_BINS]/lrelease
for(tsfile, TRANSLATIONS) {
    qmfile = $$basename(tsfile)
    qmfile ~= s,.ts$,.qm,
    qmdir = $$OUT_PWD/translations
    qmfile = $$qmdir/$$qmfile
    !exists($$qmdir) {
        system($${QMAKE_MKDIR} \"$$qmdir\")
    }
    LRELEASE_TARGET += $$qmfile
    LRELEASE_COMMAND += $$LRELEASE $$tsfile -qm $$qmfile;
}

LUPDATE = $$[QT_INSTALL_BINS]/lupdate -locations relative -no-ui-lines -no-sort
updatets.target = TRANSLATIONS
updatets.commands = $$LUPDATE $$PWD/qpacmantray.pro
releasets.target = LRELEASE_TARGET
releasets.commands = $$LRELEASE_COMMAND
releasets.depends = updatets
PRE_TARGETDEPS += TRANSLATIONS
PRE_TARGETDEPS += LRELEASE_TARGET
QMAKE_EXTRA_TARGETS += updatets releasets

transinstall.files = $$TRANS_DIR1/*.qm
transinstall.path = $$INSTALL_ROOT/$$TRANS_DIR2

desktop.files = $$PWD/qpacmantray.desktop
desktop.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/applications/

icon.files = $$PWD/pics/qpacmantray.svg
icon.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/pixmaps/

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
INSTALLS += target transinstall desktop icon

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libs/qpacman/release/ -lqpacman
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libs/qpacman/debug/ -lqpacman
else:unix: LIBS += -L$$OUT_PWD/../libs/qpacman/ -lqpacman

INCLUDEPATH += $$PWD/../libs/qpacman
DEPENDPATH += $$PWD/../libs/qpacman

DISTFILES +=

RESOURCES += \
    qpacmantray.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libs/qalpm/release/ -lqalpm
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libs/qalpm/debug/ -lqalpm
else:unix: LIBS += -L$$OUT_PWD/../libs/qalpm/ -lqalpm

INCLUDEPATH += $$PWD/../libs/qalpm
DEPENDPATH += $$PWD/../libs/qalpm
