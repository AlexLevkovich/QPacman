QT += gui widgets dbus network svg

TARGET = qpacman
TEMPLATE = lib
VERSION = 3.0.0
DEFINES += QPACMAN_LIBRARY
DEFINES += QAPPLICATION_CLASS=QApplication

CONFIG += c++11

USE_KDE {
    QT      += KConfigWidgets
    DEFINES += USE_KDE=\\\"$$USE_KDE\\\"
}

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

SOURCES += \
    actionapplier.cpp \
    categorylistview.cpp \
    categorytoolbutton.cpp \
    combotoolbutton.cpp \
    custompopuptextbrowser.cpp \
    dbrefresher.cpp \
    exclusiveactiongroup.cpp \
    filtertoolbutton.cpp \
    installbuttondelegate.cpp \
    messagedialog.cpp \
    movieicon.cpp \
    movietrayicon.cpp \
    optionaldepsdlg.cpp \
    optionsmenu.cpp \
    optionswidget.cpp \
    packagechangesdialog.cpp \
    packagedownloader.cpp \
    packageinstaller.cpp \
    packageprocessor.cpp \
    packageprovidersdialog.cpp \
    packageview.cpp \
    pacmaninfobrowser.cpp \
    pacmanwaitview.cpp \
    progressview.cpp \
    questiondialog.cpp \
    repotoolbutton.cpp \
    rootdialog.cpp \
    settingswidget.cpp \
    sheetdelegate.cpp \
    sheetwidget.cpp \
    singleapplication.cpp \
    singleapplication_p.cpp \
    static.cpp \
    textedithelper.cpp \
    textimagehandler.cpp \
    themeicons.cpp \
    treeitemsmenu.cpp \
    treemenuwidget.cpp \
    unabletoclosedialog.cpp \
    byteshumanizer.cpp \
    updatechecker.cpp \
    waitindicator.cpp \
    widgetgroup.cpp \
    widgettextobject.cpp \
    windowcenterer.cpp

HEADERS += \
    actionapplier.h \
    categorylistview.h \
    categorytoolbutton.h \
    combotoolbutton.h \
    custompopuptextbrowser.h \
    dbrefresher.h \
    exclusiveactiongroup.h \
    filtertoolbutton.h \
    installbuttondelegate.h \
    messagedialog.h \
    movieicon.h \
    movietrayicon.h \
    optionaldepsdlg.h \
    optionsmenu.h \
    optionswidget.h \
    packagechangesdialog.h \
    packagedownloader.h \
    packageinstaller.h \
    packageprocessor.h \
    packageprovidersdialog.h \
    packageview.h \
    pacmaninfobrowser.h \
    pacmanwaitview.h \
    progressview.h \
    questiondialog.h \
    repotoolbutton.h \
    rootdialog.h \
    settingswidget.h \
    sheetdelegate.h \
    sheetwidget.h \
    singleapplication.h \
    singleapplication_p.h \
    static.h \
    textedithelper.h \
    textimagehandler.h \
    themeicons.h \
    treeitemsmenu.h \
    treemenuwidget.h \
    unabletoclosedialog.h \
    byteshumanizer.h \
    updatechecker.h \
    waitindicator.h \
    widgetgroup.h \
    widgettextobject.h \
    windowcenterer.h

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

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../qpacmandbus/release/ -lqpacmandbus
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../qpacmandbus/debug/ -lqpacmandbus
else:unix:!macx: LIBS += -L$$OUT_PWD/../qpacmandbus/ -lqpacmandbus

INCLUDEPATH += $$PWD/../qpacmandbus
DEPENDPATH += $$PWD/../qpacmandbus

RESOURCES += libqpacman.qrc

FORMS += \
    optionaldepsdlg.ui \
    packagechangesdialog.ui \
    packageprovidersdialog.ui \
    rootdialog.ui \
    settingswidget.ui

transinstall.files = $$prependAll(LANGUAGES, $$TRANS_DIR1/lib$$TARGET, .qm)
transinstall.CONFIG += no_check_exist
transinstall.path = $$INSTALL_ROOT/$$TRANS_DIR2

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/
INSTALLS += target transinstall
