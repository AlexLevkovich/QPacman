#-------------------------------------------------
#
# Project created by QtCreator 2019-01-29T09:55:06
#
#-------------------------------------------------

QT       += widgets svg network

USE_KDE {
    QT      += KConfigWidgets
    DEFINES += USE_KDE=\\\"$$USE_KDE\\\"
}

TARGET = qpacman
TEMPLATE = lib
VERSION = 2.0.0
DEFINES += QPACMAN_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += depend_includepath

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman

SU_BIN = $$system(which su 2>/dev/null)
isEmpty( SU_BIN ):error( "su should be installed!!!" )
DEFINES += SU_BIN=\\\"$$SU_BIN\\\"

BASH_BIN = $$system(which bash 2>/dev/null)
isEmpty( BASH_BIN ):error( "bash should be installed!!!" )
DEFINES += BASH_BIN=\\\"$$BASH_BIN\\\"

QSU_APP1 = $$OUT_PWD/../../qsu/qsu
QSU_APP2 = $$INSTALL_PREFIX/bin/qsu
DEFINES += QSU_APP1=\\\"$$QSU_APP1\\\"
DEFINES += QSU_APP2=\\\"$$QSU_APP2\\\"

QPACMAN_APP1 = $$OUT_PWD/../../src/qpacman
QPACMAN_APP2 = $$INSTALL_PREFIX/bin/qpacman
DEFINES += QPACMAN_APP1=\\\"$$QPACMAN_APP1\\\"
DEFINES += QPACMAN_APP2=\\\"$$QPACMAN_APP2\\\"

SOURCES += \
    alpmlockingnotifier.cpp \
    categorylistview.cpp \
    categorytoolbutton.cpp \
    combotoolbutton.cpp \
    custompopuptextbrowser.cpp \
    exclusiveactiongroup.cpp \
    messagedialog.cpp \
    movieicon.cpp \
    optionaldepsdlg.cpp \
    packagechangesdialog.cpp \
    packagedownloader.cpp \
    packageprocessor.cpp \
    progressview.cpp \
    rootsyncdirupdater.cpp \
    settingswidget.cpp \
    sharedmemory.cpp \
    singleapplicationevents.cpp \
    textedithelper.cpp \
    textimagehandler.cpp \
    themeicons.cpp \
    waitindicator.cpp \
    widgetgroup.cpp \
    widgettextobject.cpp \
    windowcenterer.cpp \
    treemenuwidget.cpp \
    treeitemsmenu.cpp \
    unabletoclosedialog.cpp \
    packageretriver.cpp \
    questiondialog.cpp \
    packageinstaller.cpp \
    packageprovidersdialog.cpp \
    packageremover.cpp \
    sheetdelegate.cpp \
    sheetwidget.cpp \
    dbrefresher.cpp \
    static.cpp \
    actionapplier.cpp \
    singleapplication.cpp \
    movietrayicon.cpp \
    nobufferingprocess.cpp \
    suprocessexecutor.cpp \
    slottedeventloop.cpp \
    rootdialog.cpp \
    usualuserupdateschecker.cpp

HEADERS += \
    alpmlockingnotifier.h \
    categorylistview.h \
    custompopuptextbrowser.h \
    exclusiveactiongroup.h \
    movieicon.h \
    optionaldepsdlg.h \
    packagedownloader.h \
    packageprocessor.h \
    progressview.h \
    rootsyncdirupdater.h \
    settingswidget.h \
    sharedmemory.h \
    categorytoolbutton.h \
    combotoolbutton.h \
    messagedialog.h \
    packagechangesdialog.h \
    singleapplicationevents.h \
    textedithelper.h \
    textimagehandler.h \
    themeicons.h \
    waitindicator.h \
    widgetgroup.h \
    widgettextobject.h \
    windowcenterer.h \
    treemenuwidget.h \
    treeitemsmenu.h \
    unabletoclosedialog.h \
    packageretriver.h \
    questiondialog.h \
    packageinstaller.h \
    packageprovidersdialog.h \
    packageremover.h \
    sheetdelegate.h \
    sheetwidget.h \
    dbrefresher.h \
    static.h \
    actionapplier.h \
    singleapplication.h \
    movietrayicon.h \
    nobufferingprocess.h \
    suprocessexecutor.h \
    slottedeventloop.h \
    rootdialog.h \
    usualuserupdateschecker.h

FORMS += \
    optionaldepsdlg.ui \
    packagechangesdialog.ui \
    packageprovidersdialog.ui \
    rootdialog.ui \
    settingswidget.ui

LANGUAGES = ru be

defineReplace(prependAll) {
 for(a,$$1):result += $${2}_$${a}$$3
 return($$result)
}

TRANSLATIONS = $$prependAll(LANGUAGES, $$PWD/translations/lib$$TARGET, .ts)

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
updatets.commands = $$LUPDATE $$PWD/qpacman.pro
releasets.target = LRELEASE_TARGET
releasets.commands = $$LRELEASE_COMMAND
releasets.depends = updatets
PRE_TARGETDEPS += TRANSLATIONS
PRE_TARGETDEPS += LRELEASE_TARGET
QMAKE_EXTRA_TARGETS += updatets releasets

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../qalpm/release/ -lqalpm
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../qalpm/debug/ -lqalpm
else:unix: LIBS += -L$$OUT_PWD/../qalpm/ -lqalpm

INCLUDEPATH += $$PWD/../qalpm
DEPENDPATH += $$PWD/../qalpm

RESOURCES += \
    libqpacman.qrc

transinstall.files = $$prependAll(LANGUAGES, $$TRANS_DIR1/lib$$TARGET, .qm)
transinstall.CONFIG += no_check_exist
transinstall.path = $$INSTALL_ROOT/$$TRANS_DIR2

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/
INSTALLS += target transinstall
