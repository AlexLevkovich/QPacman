QT -= gui
QT += dbus network

TARGET = qpacman_service
CONFIG += c++11 console
CONFIG -= app_bundle

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

DEFINES += QAPPLICATION_CLASS=QCoreApplication
DEFINES += QT_DEPRECATED_WARNINGS

DBUS_ADAPTORS = qpacmanservice.xml
QDBUSXML2CPP_ADAPTOR_HEADER_FLAGS = -i alpmpackage.h -i alpmfuture.h -i qalpmtypes.h

SOURCES += \
        main.cpp \
        qpacmanservice.cpp \
        sigwatch.cpp \
        singleapplication.cpp \
        singleapplication_p.cpp

isEmpty(PACMANCONF) {
    PACMANCONF = /etc/pacman.conf
}
DEFINES += PACMANCONF=\\\"$$PACMANCONF\\\"

SYSTEMDCONFDIR = $$INSTALL_PREFIX/share/dbus-1/system.d
DEFINES += SYSTEMDCONFDIR=\\\"$$SYSTEMDCONFDIR\\\"
SYSTEMDCONFFILE = com.alexl.qt.QPacmanService.conf
DEFINES += SYSTEMDCONFFILE=\\\"$$SYSTEMDCONFFILE\\\"


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    qpacmanservice.h \
    sigwatch.h \
    singleapplication.h \
    singleapplication_p.h


!exists("$$INSTALL_PREFIX/lib/libpam.so"): error("pam package needs to be installed!")
AWK_BIN = $$system(which awk)
isEmpty(AWK_BIN): error("awk package needs to be installed!")

LIBS += -lpam

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/qalpm/release/ -lqalpm
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/qalpm/debug/ -lqalpm
else:unix: LIBS += -L$$OUT_PWD/../lib/qalpm/ -lqalpm

fileschange1.target = com.alexl.qt.QPacmanService.service
fileschange1.depends = FORCE
fileschange1.commands = cd $$PWD; cat com.alexl.qt.QPacmanService.service.orig | awk -v f="/usr/bin/" -v t="$$INSTALL_PREFIX/bin/" \'{if (substr(t,1,2) == \"//\") {t=substr(t,2)}; gsub(f,t);print \$0}\' > com.alexl.qt.QPacmanService.service
PRE_TARGETDEPS += com.alexl.qt.QPacmanService.service
QMAKE_EXTRA_TARGETS += fileschange1
fileschange2.target = qpacman.service
fileschange2.depends = FORCE
fileschange2.commands = cd $$PWD; cat qpacman.service.orig | awk -v f="/usr/bin/" -v t="$$INSTALL_PREFIX/bin/" \'{if (substr(t,1,2) == \"//\") {t=substr(t,2)}; gsub(f,t);print \$0}\' > qpacman.service
PRE_TARGETDEPS += qpacman.service
QMAKE_EXTRA_TARGETS += fileschange2

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
dbus_service.files = com.alexl.qt.QPacmanService.service

systemd_service.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/systemd/system/
systemd_service.files = qpacman.service

INSTALLS += dbus_config dbus_service systemd_service

