# QPacman
QPacman is GUI to Archlinux pacman utility.

It contains two projects:

QPacmanTray   - tray icon - checks updates, loads QPacman.

QPacman       - main program

# COMPILATION:

pkgdir & srcdir/ is for pkgbild

for subdir in $srcdir/QPacmanTray $srcdir/QPacman

do

cd $subdir

qmake "INSTALL_PREFIX=/usr" "INSTALL_ROOT=$pkgdir" CONFIG+=release CONFIG-=debug

make

make install

done

Qt verions: QT4 or QT5

Depends: 'Qt4/qt5-base' 'pacman' 'vorbis-tools' 'wget' 'coreutils' 'xz' 'tar' 'util-linux' 'psmisc' 'gawk'

