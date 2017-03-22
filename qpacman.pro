TEMPLATE = subdirs

SUBDIRS = setbuf pacmanSy \
    qpacmankill \
    pacmanrmcache \
    libqpacman

equals(QPACMAN_CLIENT, "ON") | isEmpty(QPACMAN_CLIENT) {
    SUBDIRS += QPacman
    QPacman.depends = setbuf pacmanSy libqpacman pacmanrmcache qpacmankill
}
equals(QPACMAN_TRAY, "ON") | isEmpty(QPACMAN_TRAY) {
    SUBDIRS += QPacmanTray
    QPacmanTray.depends = setbuf pacmanSy libqpacman qpacmankill
}

RESOURCES += \
    libqpacman/qpacman.qrc

