#-------------------------------------------------
#
# Project created by QtCreator 2017-02-19T09:12:49
#
#-------------------------------------------------

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qpacman
TEMPLATE = lib

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/qpacman

DEFINES += INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"
DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"

SO_BIN1=$$INSTALL_ROOT/$$INSTALL_PREFIX/lib/libsetbuf.so
SO_BIN2=$$PWD/../bin/libsetbuf.so
DEFINES += SO_BIN1=\\\"$$SO_BIN1\\\"
DEFINES += SO_BIN2=\\\"$$SO_BIN2\\\"

TAR_BIN = $$system(which tar 2>/dev/null)
isEmpty( TAR_BIN ):error( "tar should be installed!!!" )
DEFINES += TAR_BIN=\\\"$$TAR_BIN\\\"

WGET_BIN = $$system(which wget 2>/dev/null)
isEmpty( WGET_BIN ):error( "wget should be installed!!!" )
DEFINES += WGET_BIN=\\\"$$WGET_BIN\\\"

SU_BIN = $$system(which su 2>/dev/null)
isEmpty( SU_BIN ):error( "su should be installed!!!" )
DEFINES += SU_BIN=\\\"$$SU_BIN\\\"

PACMAN_BIN = $$system(which pacman 2>/dev/null)
isEmpty( PACMAN_BIN ):error( "pacman should be installed!!!" )
DEFINES += PACMAN_BIN=\\\"$$PACMAN_BIN\\\"

PACMANSY_BIN = $$INSTALL_PREFIX/bin/pacmanSy
DEFINES += PACMANSY_BIN=\\\"$$PACMANSY_BIN\\\"

PACMANRMCACHE_BIN = $$INSTALL_PREFIX/bin/pacmanrmcache
DEFINES += PACMANRMCACHE_BIN=\\\"$$PACMANRMCACHE_BIN\\\"

QPACMANKILL_BIN = $$INSTALL_PREFIX/bin/qpacmankill
DEFINES += QPACMANKILL_BIN=\\\"$$QPACMANKILL_BIN\\\"

BASH_BIN = $$system(which bash 2>/dev/null)
isEmpty( BASH_BIN ):error( "bash should be installed!!!" )
DEFINES += BASH_BIN=\\\"$$BASH_BIN\\\"

CAT_BIN = $$system(which cat 2>/dev/null)
isEmpty( CAT_BIN ):error( "cat should be installed!!!" )
DEFINES += CAT_BIN=\\\"$$CAT_BIN\\\"

DEFINES += LIBQPACMAN_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS

lessThan(QT_MAJOR_VERSION, 5): {
SOURCES += qlockfile.cpp \
           qlockfile_unix.cpp
HEADERS += qlockfile.h \
           qlockfile_p.h
}

SOURCES += \
    libqpacman.cpp \
    byteshumanizer.cpp \
    dbrefreshdialog.cpp \
    pacmansheetwidget.cpp \
    pacmansimpleupdatesreader.cpp \
    pacmanupdatepackagesreader.cpp \
    removeprogressdialog.cpp \
    logwindow.cpp \
    messagedialog.cpp \
    packagechangesdialog.cpp \
    pacmancachecleaner.cpp \
    pacmandbrefresher.cpp \
    pacmanentry.cpp \
    pacmanfilefinder.cpp \
    pacmanfilepackageinforeader.cpp \
    pacmanfileslistreader.cpp \
    pacmaninstalllocalpackagesreader.cpp \
    pacmaninstallpackagesreader.cpp \
    pacmanprocessreader.cpp \
    pacmanprovidersdialog.cpp \
    pacmanremovepackagesreader.cpp \
    pacmanrepositoryreader.cpp \
    pacmansetupinforeader.cpp \
    pacmansheetdelegate.cpp \
    filesdownloaddialog.cpp \
    installfilesprogressloop.cpp \
    installprogressdialog.cpp \
    installprogressloop.cpp \
    suchecker.cpp \
    wraplabel.cpp \
    rootdialog.cpp \
    removeprogressloop.cpp \
    categorytoolbutton.cpp \
    combotoolbutton.cpp \
    static.cpp \
    confsettings.cpp \
    treeitemsmenu.cpp \
    treemenuwidget.cpp \
    pacmanpackagereasonchanger.cpp \
    windowcenterer.cpp

HEADERS += \
    libqpacman.h\
    libqpacman_global.h \
    byteshumanizer.h \
    dbrefreshdialog.h \
    errordialog.h \
    pacmansheetwidget.h \
    pacmansimpleupdatesreader.h \
    pacmanupdatepackagesreader.h \
    logwindow.h \
    messagedialog.h \
    packagechangesdialog.h \
    pacmancachecleaner.h \
    pacmandbrefresher.h \
    pacmanentry.h \
    pacmanfilefinder.h \
    pacmanfilepackageinforeader.h \
    pacmanfileslistreader.h \
    pacmaninstalllocalpackagesreader.h \
    pacmaninstallpackagesreader.h \
    pacmanprocessreader.h \
    pacmanprovidersdialog.h \
    pacmanremovepackagesreader.h \
    pacmanrepositoryreader.h \
    pacmansetupinforeader.h \
    pacmansheetdelegate.h \
    filesdownloaddialog.h \
    installfilesprogressloop.h \
    installprogressdialog.h \
    installprogressloop.h \
    wraplabel.h \
    suchecker.h \
    rootdialog.h \
    removeprogressloop.h \
    removeprogressdialog.h \
    categorytoolbutton.h \
    combotoolbutton.h \
    static.h \
    confsettings.h \
    treeitemsmenu.h \
    treemenuwidget.h \
    pacmanpackagereasonchanger.h \
    windowcenterer.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    logwindow.ui \
    packagechangesdialog.ui \
    pacmanprovidersdialog.ui \
    filesdownloaddialog.ui \
    rootdialog.ui

RESOURCES += \
    libqpacman.qrc

TRANSLATIONS = $$PWD/translations/libqpacman_ru.ts \
               $$PWD/translations/libqpacman_be.ts

LUPDATE = $$[QT_INSTALL_BINS]/lupdate -locations relative -no-ui-lines -no-sort
LRELEASE = $$[QT_INSTALL_BINS]/lrelease

updatets.files = TRANSLATIONS
updatets.commands = $$LUPDATE $$PWD/libqpacman.pro

QMAKE_EXTRA_TARGETS += updatets

updateqm.depends = updatets
updateqm.input = TRANSLATIONS
updateqm.output = translations/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.name = LRELEASE ${QMAKE_FILE_IN}
updateqm.variable_out = PRE_TARGETDEPS
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm

INSTALL_TRANSLATIONS += $$TRANS_DIR1/libqpacman_ru.qm $$TRANS_DIR1/libqpacman_be.qm
transinstall.files = $$INSTALL_TRANSLATIONS
transinstall.path = $$TRANS_DIR2

INSTALLS += transinstall

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/
INSTALLS += target

