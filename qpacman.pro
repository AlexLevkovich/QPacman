TEMPLATE = subdirs

SUBDIRS = setbuf pacmanSy

equals(QPACMAN_CLIENT, "ON") | isEmpty(QPACMAN_CLIENT) {
    SUBDIRS += QPacman
    QPacman.depends = setbuf pacmanSy
}
equals(QPACMAN_TRAY, "ON") | isEmpty(QPACMAN_TRAY) {
    SUBDIRS += QPacmanTray
    QPacmanTray.depends = setbuf pacmanSy
}

