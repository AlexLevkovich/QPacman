TEMPLATE = lib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += plugin

SOURCES += \
    libstdbuf.c

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/
INSTALLS += target

createsolnk.files = $$OUT_PWD/libsetbuf.so
createsolnk.commands = mkdir -p $$PWD/../bin; cd $$PWD/../bin; ln -sf $$OUT_PWD/libsetbuf.so libsetbuf.so
QMAKE_EXTRA_TARGETS += createsolnk
POST_TARGETDEPS += createsolnk

