# QPACMAN v3.0
**qpacman** is GUI to Archlinux pacman's alpm lib.  

It contains four subprojects:  

**qpacman_service** - dbus service  
**qpacman** - main program  
**qpacmantray**  - tray icon - checks updates, loads qpacman  
**qpacmanlocal** - program to install the local packages  
**libs** - contains three libraries: **qpacman**, **qalpm** and **qpacmandbus**  

# COMPILATION:

**pkgdir** is for pkgbild (ignore it if you are going to compile manually)  
**srcdir** is the main dir with qpacman.pro  

cd $srcdir  
qmake "INSTALL_PREFIX=/usr" "INSTALL_ROOT=$pkgdir" CONFIG+=release CONFIG-=debug  
make  
make install  

You can use **CONFIG+=USE_KDE** in qmake arguments if you need to use the extended KDE color scheme  
Be sure that you've added the one more dependency to PKGBUILD: **kconfigwidgets**  
You can use **CONFIG+=USER_AUTH=0** in qmake arguments if you need to use root authorization for the operations that modify the system


**QT5 only**  

Depends: '**Qt >= 5: core,network,gui,dbus,multimedia,widgets,svg**', '**pacman**', '**pam**', '**libarchive**' '**binutils**' '**appstream-qt**' '**archlinux-appstream-data**' and '**gawk**' as a makedepend. 
  

