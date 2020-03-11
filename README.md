# QPACMAN v. 2.0
**qpacman** is GUI to Archlinux pacman utility.  

It contains four subprojects:  

**qpacman** - main program  
**qpacmantray**  - tray icon - checks updates, loads qpacman  
**qsu** - wrapper to execute qpacman under root  
**libs** - contains two libraries: **qpacman** and **qalpm**  

# COMPILATION:

**pkgdir** is for pkgbild (ignore it if you are going to compile manually)  
**srcdir** is the main dir with qpacman.pro  
If you don't want to install **qpacman** app then you have to add QPACMAN_CLIENT=OFF after qmake.  
If you don't want to install **qpacmantray** app then you have to add QPACMAN_TRAY=OFF after qmake.  

cd $srcdir  
qmake "INSTALL_PREFIX=/usr" "INSTALL_ROOT=$pkgdir" CONFIG+=release CONFIG-=debug  
make  
make install  

**QT5 only**  

Depends: '**Qt >= 5: core,network,gui,multimedia,widgets,svg**', '**pacman**', '**pam**', '**libarchive**'  

