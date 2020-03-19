#-------------------------------------------------
#
# Project created by QtCreator 2019-01-29T09:53:47
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qpacman
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

QPACMAN_VERSION = 2.0-alpm
DEFINES += QPACMAN_VERSION=\\\"$$QPACMAN_VERSION\\\"

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

isEmpty(PACMANCONF) {
    PACMANCONF = /etc/pacman.conf
}
DEFINES += PACMANCONF=\\\"$$PACMANCONF\\\"

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman
TRANS_DIR3 = $$OUT_PWD/../libs/qpacman/translations
TRANS_DIR4 = $$OUT_PWD/../libs/qalpm/translations

DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"
DEFINES += TRANS_DIR3=\\\"$$TRANS_DIR3\\\"
DEFINES += TRANS_DIR4=\\\"$$TRANS_DIR4\\\"

CONFIG += c++11
CONFIG += depend_includepath

SOURCES += \
    infoview.cpp \
        main.cpp \
        mainwindow.cpp \
    fileslistwidget.cpp \
    filtertoolbutton.cpp \
    installbuttondelegate.cpp \
    optionsmenu.cpp \
    optionswidget.cpp \
    pacmaninfobrowser.cpp \
    pacmanitemmodel.cpp \
    pacmantoolbar.cpp \
    pacmanview.cpp \
    pacmanwaitview.cpp \
    repotoolbutton.cpp \
    searchlineedit.cpp \
    searchwidget.cpp \
    localpackagemainwindow.cpp \
    pacmanlocalpackageslistdelegate.cpp \
    pacmansimpleitemmodel.cpp \
    qpacmanapplication.cpp

HEADERS += \
    infoview.h \
        mainwindow.h \
    fileslistwidget.h \
    filtertoolbutton.h \
    installbuttondelegate.h \
    optionsmenu.h \
    optionswidget.h \
    pacmaninfobrowser.h \
    pacmanitemmodel.h \
    pacmantoolbar.h \
    pacmanview.h \
    pacmanwaitview.h \
    repotoolbutton.h \
    searchlineedit.h \
    searchwidget.h \
    localpackagemainwindow.h \
    pacmanlocalpackageslistdelegate.h \
    pacmansimpleitemmodel.h \
    qpacmanapplication.h

FORMS += \
        mainwindow.ui \
    searchwidget.ui \
    localpackagemainwindow.ui

RESOURCES += \
    qpacman.qrc

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
updatets.commands = $$LUPDATE $$PWD/src.pro
releasets.target = LRELEASE_TARGET
releasets.commands = $$LRELEASE_COMMAND
releasets.depends = updatets
PRE_TARGETDEPS += TRANSLATIONS
PRE_TARGETDEPS += LRELEASE_TARGET
QMAKE_EXTRA_TARGETS += updatets releasets

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libs/qpacman/release/ -lqpacman
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libs/qpacman/debug/ -lqpacman
else:unix: LIBS += -L$$OUT_PWD/../libs/qpacman/ -lqpacman

INCLUDEPATH += $$_PRO_FILE_PWD_/../libs/qpacman
DEPENDPATH += $$_PRO_FILE_PWD_/../libs/qpacman

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libs/qalpm/release/ -lqalpm
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libs/qalpm/debug/ -lqalpm
else:unix: LIBS += -L$$OUT_PWD/../libs/qalpm/ -lqalpm

INCLUDEPATH += $$_PRO_FILE_PWD_/../libs/qalpm
DEPENDPATH += $$_PRO_FILE_PWD_/../libs/qalpm

transinstall.files = $$prependAll(LANGUAGES, $$TRANS_DIR1/$$TARGET, .qm)
transinstall.CONFIG += no_check_exist
transinstall.path = $$INSTALL_ROOT/$$TRANS_DIR2

icon.files = $$PWD/pics/qpacman.svg
icon.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/pixmaps/

desktop.files = $$PWD/qpacman.desktop $$PWD/qpacman_view.desktop
desktop.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/applications/

kde.files = $$PWD/qpacmanKDEService.desktop
greaterThan(QT_MAJOR_VERSION, 4): kde.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/kservices5/
lessThan(QT_MAJOR_VERSION, 5): kde.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/kde4/services/ServiceMenus/

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
INSTALLS += target transinstall icon desktop kde
