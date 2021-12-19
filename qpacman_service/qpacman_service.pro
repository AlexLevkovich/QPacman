QT -= gui
QT += dbus network

TARGET = qpacman_service
CONFIG += c++11 console
CONFIG -= app_bundle

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}
DEFINES += INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"

isEmpty(USER_AUTH) {
    USER_AUTH = 1
}
DEFINES += USER_AUTH=$$USER_AUTH

DEFINES += QAPPLICATION_CLASS=QCoreApplication
DEFINES += QT_DEPRECATED_WARNINGS

DBUS_ADAPTORS = qpacmanservice.xml
QDBUSXML2CPP_ADAPTOR_HEADER_FLAGS = -i alpmpackage.h -i alpmfuture.h -i qalpmtypes.h -i dbusstring.h

TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman
TRANS_DIR4 = $$OUT_PWD/../lib/qalpm/translations

DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"
DEFINES += TRANS_DIR4=\\\"$$TRANS_DIR4\\\"

SOURCES += \
        actionapplier.cpp \
        dbusstring.cpp \
        main.cpp \
        qpacmanservice.cpp \
        sigwatch.cpp \
        singleapplication.cpp \
        singleapplication_p.cpp \
        stacktracer.cpp

isEmpty(PACMANCONF) {
    PACMANCONF = /etc/pacman.conf
}
DEFINES += PACMANCONF=\\\"$$PACMANCONF\\\"

SYSTEMDCONFDIR = $$INSTALL_PREFIX/share/dbus-1/system.d
DEFINES += SYSTEMDCONFDIR=\\\"$$SYSTEMDCONFDIR\\\"
SYSTEMDCONFFILEBASE = com.alexl.qt.QPacmanService
DEFINES += SYSTEMDCONFFILEBASE=\\\"$$SYSTEMDCONFFILEBASE\\\"
OWNPKGNAME = qpacman
DEFINES += OWNPKGNAME=\\\"$$OWNPKGNAME\\\"

LOGDIR = /var/log
DEFINES += LOGDIR=\\\"$$LOGDIR\\\"

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    actionapplier.h \
    dbusstring.h \
    qpacmanservice.h \
    sigwatch.h \
    singleapplication.h \
    singleapplication_p.h \
    stacktracer.h


!exists("$$INSTALL_PREFIX/lib/libpam.so"): error("pam package needs to be installed!")
AWK_BIN = $$system(which awk)
isEmpty(AWK_BIN): error("awk package needs to be installed!")
!exists("$$INSTALL_PREFIX/lib/libbfd.so"): error("binutils package needs to be installed!")

LIBS += -lpam -lbfd

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/qalpm/release/ -lqalpm
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/qalpm/debug/ -lqalpm
else:unix: LIBS += -L$$OUT_PWD/../lib/qalpm/ -lqalpm

fileschange1.input = com.alexl.qt.QPacmanService.service.in
fileschange1.output = $$OUT_PWD/com.alexl.qt.QPacmanService.service
fileschange2.input = qpacman.service.in
fileschange2.output = $$OUT_PWD/qpacman.service
QMAKE_SUBSTITUTES += fileschange1 fileschange2

INCLUDEPATH += $$PWD/../lib/qalpm
DEPENDPATH += $$PWD/../lib/qalpm

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/
INSTALLS += target

DISTFILES += \
    com.alexl.qt.QPacmanService.conf \
    com.alexl.qt.QPacmanService.service \
    qpacman.service

dbus_config.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/dbus-1/system.d/
dbus_config.files = com.alexl.qt.QPacmanService.conf

dbus_service.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/dbus-1/system-services/
dbus_service.files = $$OUT_PWD/com.alexl.qt.QPacmanService.service

systemd_service.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/systemd/system/
systemd_service.files = $$OUT_PWD/qpacman.service

INSTALLS += dbus_config dbus_service systemd_service

