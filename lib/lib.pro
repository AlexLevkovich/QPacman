#-------------------------------------------------
#
# Project created by QtCreator 2019-01-29T09:15:44
#
#-------------------------------------------------

CONFIG += c++11
TEMPLATE = subdirs
SUBDIRS += qalpm \
           qpacman \
           qpacmandbus 

qpacmandbus.depends = qalpm
qpacman.depends = qpacmandbus
