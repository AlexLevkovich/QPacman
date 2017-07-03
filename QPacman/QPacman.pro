#-------------------------------------------------
#
# Project created by QtCreator 2013-11-20T09:20:31
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QPacman
TEMPLATE = app

INCLUDEPATH += .
INCLUDEPATH += $$PWD/../libqpacman
LIBS += -L$$OUT_PWD/../libqpacman -lqpacman
QMAKE_LFLAGS += "-Wl,-rpath,\'\$$OUT_PWD/../libqpacman\'"

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

PATH = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib:$$PWD/..:$$PATH
export(PATH)

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR3 = $$OUT_PWD/../libqpacman/translations
TRANS_DIR2 = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/qpacman

DEFINES += INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"
DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"
DEFINES += TRANS_DIR3=\\\"$$TRANS_DIR3\\\"

SOURCES += main.cpp\
        mainwindow.cpp \
    searchwidget.cpp \
    filtertoolbutton.cpp \
    searchlineedit.cpp \
    fileslistwidget.cpp \
    toolbarwidget.cpp \
    version.c \
    toolbarrightwidget.cpp \
    pacmaninfobrowser.cpp \
    pacmanitemmodel.cpp \
    pacmantoolbar.cpp \
    pacmanview.cpp \
    localpackagemainwindow.cpp \
    pacmansimpleitemmodel.cpp \
    installbuttondelegate.cpp \
    repotoolbutton.cpp \
    pacmanlocalpackageslistdelegate.cpp \
    pacmaninfobrowserdocument.cpp \
    pacmanhelpdialog.cpp \
    pacmanwaitview.cpp

HEADERS  += mainwindow.h \
    searchwidget.h \
    filtertoolbutton.h \
    searchlineedit.h \
    fileslistwidget.h \
    toolbarwidget.h \
    toolbarrightwidget.h \
    pacmaninfobrowser.h \
    pacmanitemmodel.h \
    pacmantoolbar.h \
    pacmanview.h \
    localpackagemainwindow.h \
    pacmansimpleitemmodel.h \
    installbuttondelegate.h \
    repotoolbutton.h \
    pacmanlocalpackageslistdelegate.h \
    pacmaninfobrowserdocument.h \
    pacmanhelpdialog.h \
    pacmanwaitview.h


FORMS    += mainwindow.ui \
    searchwidget.ui \
    toolbarrightwidget.ui \
    localpackagemainwindow.ui \
    pacmanhelpdialog.ui

RESOURCES += qpacman.qrc

TRANSLATIONS = $$PWD/translations/qpacman_ru.ts \
               $$PWD/translations/qpacman_be.ts

LUPDATE = $$[QT_INSTALL_BINS]/lupdate -locations relative -no-ui-lines -no-sort
LRELEASE = $$[QT_INSTALL_BINS]/lrelease

updatets.files = TRANSLATIONS
updatets.commands = $$LUPDATE $$PWD/QPacman.pro

QMAKE_EXTRA_TARGETS += updatets

updateqm.depends = updatets
updateqm.input = TRANSLATIONS
updateqm.output = translations/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.name = LRELEASE ${QMAKE_FILE_IN}
updateqm.variable_out = PRE_TARGETDEPS
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm

TRANSLATIONS_OUT = $$TRANS_DIR1/qpacman_ru.qm \
                   $$TRANS_DIR1/qpacman_be.qm

qmtr.files = $$TRANSLATIONS_OUT
qmtr.path = $$TRANS_DIR2

desktop.files = $$PWD/QPacman.desktop
desktop.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/applications/

icon.files = pics/Pacman-arch_logo.png
icon.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/pixmaps/

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/

kde.files = QPacmanKDEService.desktop
greaterThan(QT_MAJOR_VERSION, 4): kde.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/kservices5/
lessThan(QT_MAJOR_VERSION, 5): kde.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/kde4/services/ServiceMenus/

INSTALLS += target qmtr desktop icon kde
