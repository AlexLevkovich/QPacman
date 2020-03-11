#-------------------------------------------------
#
# Project created by QtCreator 2019-01-29T09:15:44
#
#-------------------------------------------------

CONFIG += c++11
TEMPLATE = subdirs
SUBDIRS += libs \
           qsu

equals(QPACMAN_CLIENT, "ON") | isEmpty(QPACMAN_CLIENT) {
    SUBDIRS += src
    src.depends = libs
}
equals(QPACMAN_TRAY, "ON") | isEmpty(QPACMAN_TRAY) {
    SUBDIRS += qpacmantray
    qpacmantray.depends = libs
}

qsu.depends = libs
