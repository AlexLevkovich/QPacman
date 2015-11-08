# QPacman
QPacman is GUI to Archlinux pacman utility.

It contains three projects:

QPacmanServer - dbus server program (works under root) - works with pacman utility.
QPacmanTray   - tray icon - checks updates, loads QPacman.
QPacman       - main program

# COMPILATION:

pkgdir & srcdir/ is for pkgbild
for subdir in $srcdir/QPacmanServer $srcdir/QPacmanTray $srcdir/QPacman
do
cd $subdir
qmake "INSTALL_PREFIX=/usr" "INSTALL_ROOT=$pkgdir" CONFIG+=release CONFIG-=debug
make
make install
done

Qt verions: QT4 or QT5

Depends: 'Qt >= 4', 'systemd' 'pacman' 'vorbis-tools' 'wget' 'procps-ng' 'coreutils'

