QT -= gui
QT += dbus network

TARGET = qpacmandbus
TEMPLATE = lib
VERSION = 1.0.0
DEFINES += QPACMANDBUS_LIBRARY

CONFIG += c++11

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#qdbusxmltocpp.target = qpacmanservice_interface.h
#qdbusxmltocpp.depends = FORCE
#qdbusxmltocpp.commands = cd $$PWD; $$[QT_INSTALL_BINS]/qdbusxml2cpp -i alpmpackage.h -i qalpmtypes.h -p qpacmanservice_interface.h: $$PWD/../qpacman_service/qpacmanservice.xml; $$[QT_INSTALL_BINS]/qdbusxml2cpp -i qpacmanservice_interface.h -p :qpacmanservice_interface.cpp $$PWD/../qpacman_service/qpacmanservice.xml
#PRE_TARGETDEPS += qpacmanservice_interface.h
#QMAKE_EXTRA_TARGETS += qdbusxmltocpp

DBUS_INTERFACES = $$PWD/../../qpacman_service/qpacmanservice.xml
QDBUSXML2CPP_INTERFACE_HEADER_FLAGS = -i alpmpackage.h -i qalpmtypes.h -i dbusstring.h

SOURCES += \
    dbusstring.cpp \
    alpmpackage.cpp \
    libalpm.cpp

HEADERS += \
    dbusstring.h \
    alpmpackage.h \
    libalpm.h \
    qalpmtypes.h

INCLUDEPATH += $$OUT_PWD

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/
INSTALLS += target


