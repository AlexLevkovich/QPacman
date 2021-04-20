#-------------------------------------------------
#
# Project created by QtCreator 2019-01-29T09:15:44
#
#-------------------------------------------------

CONFIG += c++11
TEMPLATE = subdirs
SUBDIRS += lib \
           qpacman \
           qpacman_service \
           qpacmanlocal \
           qpacmantray

qpacman_service.depends = lib
qpacman.depends = lib
qpacmantray.depends = lib
qpacmanlocal.depends = lib
