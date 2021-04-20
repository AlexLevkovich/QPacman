QT      += core gui dbus widgets network multimedia

DEFINES += QAPPLICATION_CLASS=QApplication

CONFIG += c++11

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman
TRANS_DIR3 = $$OUT_PWD/../lib/qpacman/translations
TRANS_DIR4 = $$OUT_PWD/../lib/qalpm/translations

DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"
DEFINES += TRANS_DIR3=\\\"$$TRANS_DIR3\\\"
DEFINES += TRANS_DIR4=\\\"$$TRANS_DIR4\\\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    alpmoptionswidget.cpp \
    main.cpp \
    qpacmantrayapplication.cpp \
    qpacmantrayicon.cpp \
    trayoptionswidget.cpp \
    traypreferences.cpp

HEADERS += \
    alpmoptionswidget.h \
    qpacmantrayapplication.h \
    qpacmantrayicon.h \
    trayoptionswidget.h \
    traypreferences.h

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

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/qpacmandbus/release/ -lqpacmandbus
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/qpacmandbus/debug/ -lqpacmandbus
else:unix:!macx: LIBS += -L$$OUT_PWD/../lib/qpacmandbus/ -lqpacmandbus

INCLUDEPATH += $$PWD/../lib/qpacmandbus
DEPENDPATH += $$PWD/../lib/qpacmandbus

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/qpacman/release/ -lqpacman
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/qpacman/debug/ -lqpacman
else:unix: LIBS += -L$$OUT_PWD/../lib/qpacman/ -lqpacman

INCLUDEPATH += $$PWD/../lib/qpacman
DEPENDPATH += $$PWD/../lib/qpacman

RESOURCES += \
    qpacmantray.qrc

transinstall.files = $$prependAll(LANGUAGES, $$TRANS_DIR1/$$TARGET, .qm)
transinstall.CONFIG += no_check_exist
transinstall.path = $$INSTALL_ROOT/$$TRANS_DIR2

desktop.files = $$PWD/qpacmantray.desktop
desktop.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/applications/

xdg.files = $$PWD/qpacmantray_xdg.desktop
xdg.path = $$INSTALL_ROOT/etc/xdg/autostart/

icon.files = $$PWD/pics/qpacmantray.svg
icon.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/pixmaps/

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
INSTALLS += target transinstall desktop icon xdg
