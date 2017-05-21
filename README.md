# QPacman
QPacman is GUI to Archlinux pacman utility.

It contains four subprojects:

QPacmanTray   - tray icon - checks updates, loads QPacman.
QPacman       - main program
setbuf        - *.so library for LD_PRELOAD. It is needed to disable QProcess caching.
pacmanSy      - suid wrapper for pacman -Sy to start it with usual user permission.

# COMPILATION:

pkgdir is for pkgbild (ignote it you compile manually)
srcdir is dir with qpacman.pro
If you does not want to install QPacman app then you have to add QPACMAN_CLIENT=OFF after qmake.
If you does not want to install QPacmanTray app then you have to add QPACMAN_TRAY=OFF after qmake.

cd $srcdir
qmake "INSTALL_PREFIX=/usr" "INSTALL_ROOT=$pkgdir" CONFIG+=release CONFIG-=debug
make
make install

Qt verions: QT4 or QT5

Depends: 'Qt >= 4', 'pacman' 'vorbis-tools' 'wget' 'coreutils' 'xz' 'tar' 'util-linux'

