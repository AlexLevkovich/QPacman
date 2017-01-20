#-------------------------------------------------
#
# Project created by QtCreator 2013-11-20T09:20:31
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QPacman
TEMPLATE = app

LIBS += -lcrypt

DEFINES += PACMANENTRY
DEFINES += IS_QPACMAN_CLIENT
INCLUDEPATH += .

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr/local
}

PATH = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib:$$PWD/..:$$PATH
export(PATH)

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman

DEFINES += INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"
DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"

SO_BIN1=$$INSTALL_PREFIX/lib/libsetbuf.so
SO_BIN2=$$PWD/../bin/libsetbuf.so
DEFINES += SO_BIN1=\\\"$$SO_BIN1\\\"
DEFINES += SO_BIN2=\\\"$$SO_BIN2\\\"

TAR_BIN = $$system(which tar 2>/dev/null)
isEmpty( TAR_BIN ):error( "tar should be installed!!!" )
DEFINES += TAR_BIN=\\\"$$TAR_BIN\\\"

WGET_BIN = $$system(which wget 2>/dev/null)
isEmpty( WGET_BIN ):error( "wget should be installed!!!" )
DEFINES += WGET_BIN=\\\"$$WGET_BIN\\\"

RM_BIN = $$system(which rm 2>/dev/null)
isEmpty( RM_BIN ):error( "rm should be installed!!!" )
DEFINES += RM_BIN=\\\"$$RM_BIN\\\"

SU_BIN = $$system(which su 2>/dev/null)
isEmpty( SU_BIN ):error( "su should be installed!!!" )
DEFINES += SU_BIN=\\\"$$SU_BIN\\\"

PACMAN_BIN = $$system(which pacman 2>/dev/null)
isEmpty( PACMAN_BIN ):error( "pacman should be installed!!!" )
DEFINES += PACMAN_BIN=\\\"$$PACMAN_BIN\\\"

PACMANSY_BIN = $$INSTALL_PREFIX/bin/pacmanSy
DEFINES += PACMANSY_BIN=\\\"$$PACMANSY_BIN\\\"

BASH_BIN = $$system(which bash 2>/dev/null)
isEmpty( BASH_BIN ):error( "bash should be installed!!!" )
DEFINES += BASH_BIN=\\\"$$BASH_BIN\\\"

KILL_BIN = $$system(which kill 2>/dev/null)
isEmpty( KILL_BIN ):error( "kill should be installed!!!" )
DEFINES += KILL_BIN=\\\"$$KILL_BIN\\\"

CAT_BIN = $$system(which cat 2>/dev/null)
isEmpty( CAT_BIN ):error( "cat should be installed!!!" )
DEFINES += CAT_BIN=\\\"$$CAT_BIN\\\"

PSTREE_BIN = $$system(which pstree 2>/dev/null)
isEmpty( PSTREE_BIN ):error( "pstree should be installed!!!" )
DEFINES += PSTREE_BIN=\\\"$$PSTREE_BIN\\\"

AWK_BIN = $$system(which awk 2>/dev/null)
isEmpty( AWK_BIN ):error( "awk should be installed!!!" )
DEFINES += AWK_BIN=\\\"$$AWK_BIN\\\"

TR_BIN = $$system(which tr 2>/dev/null)
isEmpty( TR_BIN ):error( "tr should be installed!!!" )
DEFINES += TR_BIN=\\\"$$TR_BIN\\\"

lessThan(QT_MAJOR_VERSION, 5): {
SOURCES += qlockfile.cpp \
           qlockfile_unix.cpp
HEADERS += qlockfile.h \
           qlockfile_p.h
}

SOURCES += main.cpp\
        mainwindow.cpp \
    searchwidget.cpp \
    combotoolbutton.cpp \
    categorytoolbutton.cpp \
    filtertoolbutton.cpp \
    searchlineedit.cpp \
    fileslistwidget.cpp \
    toolbarwidget.cpp \
    version.c \
    dbrefreshdialog.cpp \
    errordialog.cpp \
    pacmanentry.cpp \
    packagechangesdialog.cpp \
    removeprogressdialog.cpp \
    installprogressdialog.cpp \
    logwindow.cpp \
    toolbarrightwidget.cpp \
    pacmancachecleaner.cpp \
    pacmandbrefresher.cpp \
    pacmanfilepackageinforeader.cpp \
    pacmanfileslistreader.cpp \
    pacmaninfobrowser.cpp \
    pacmaninstallpackagesreader.cpp \
    pacmanitemmodel.cpp \
    pacmanprocessreader.cpp \
    pacmanremovepackagesreader.cpp \
    pacmanrepositoryreader.cpp \
    pacmantoolbar.cpp \
    pacmanview.cpp \
    pacmanpackagereasonchanger.cpp \
    localpackagemainwindow.cpp \
    pacmansimpleitemmodel.cpp \
    pacmaninstalllocalpackagesreader.cpp \
    installbuttondelegate.cpp \
    rootdialog.cpp \
    static.cpp \
    repotoolbutton.cpp \
    pacmanlocalpackageslistdelegate.cpp \
    posterrordlg.cpp \
    pacmanprovidersdialog.cpp \
    pacmansheetdelegate.cpp \
    pacmansheetwidget.cpp \
    pacmaninfobrowserdocument.cpp \
    treeitemsmenu.cpp \
    treemenuwidget.cpp \
    byteshumanizer.cpp \
    pacmanhelpdialog.cpp \
    pacmanwaitview.cpp \
    filesdownloaddialog.cpp \
    messagedialog.cpp \
    installprogressloop.cpp \
    installfilesprogressloop.cpp \
    removeprogressloop.cpp \
    confsettings.cpp \
    suchecker.cpp \
    pacmansetupinforeader.cpp

HEADERS  += mainwindow.h \
    searchwidget.h \
    combotoolbutton.h \
    categorytoolbutton.h \
    filtertoolbutton.h \
    searchlineedit.h \
    fileslistwidget.h \
    toolbarwidget.h \
    dbrefreshdialog.h \
    errordialog.h \
    pacmanentry.h \
    packagechangesdialog.h \
    removeprogressdialog.h \
    installprogressdialog.h \
    logwindow.h \
    toolbarrightwidget.h \
    pacmancachecleaner.h \
    pacmandbrefresher.h \
    pacmanfilepackageinforeader.h \
    pacmanfileslistreader.h \
    pacmaninfobrowser.h \
    pacmaninstallpackagesreader.h \
    pacmanitemmodel.h \
    pacmanprocessreader.h \
    pacmanremovepackagesreader.h \
    pacmanrepositoryreader.h \
    pacmantoolbar.h \
    pacmanview.h \
    pacmanpackagereasonchanger.h \
    localpackagemainwindow.h \
    pacmansimpleitemmodel.h \
    pacmaninstalllocalpackagesreader.h \
    installbuttondelegate.h \
    rootdialog.h \
    static.h \
    repotoolbutton.h \
    pacmanlocalpackageslistdelegate.h \
    posterrordlg.h \
    pacmanprovidersdialog.h \
    pacmansheetdelegate.h \
    pacmansheetwidget.h \
    pacmaninfobrowserdocument.h \
    treeitemsmenu.h \
    treemenuwidget.h \
    byteshumanizer.h \
    pacmanhelpdialog.h \
    pacmanwaitview.h \
    filesdownloaddialog.h \
    messagedialog.h \
    installprogressloop.h \
    installfilesprogressloop.h \
    removeprogressloop.h \
    confsettings.h \
    suchecker.h \
    pacmansetupinforeader.h

FORMS    += mainwindow.ui \
    searchwidget.ui \
    rootdialog.ui \
    packagechangesdialog.ui \
    logwindow.ui \
    toolbarrightwidget.ui \
    localpackagemainwindow.ui \
    pacmanprovidersdialog.ui \
    pacmanhelpdialog.ui \
    filesdownloaddialog.ui

RESOURCES += qpacman.qrc

TRANSLATIONS = $$PWD/translations/qpacman_ru.ts \
               $$PWD/translations/qpacman_be.ts

LUPDATE = $$[QT_INSTALL_BINS]/lupdate -locations relative -no-ui-lines -no-sort
LRELEASE = $$[QT_INSTALL_BINS]/lrelease

updatets.files = TRANSLATIONS
updatets.commands = $$LUPDATE $$PWD/QPacman.pro

QMAKE_EXTRA_TARGETS += updatets

updateqm.depends = updatets
updateqm.input = TRANSLATIONS
updateqm.output = translations/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.name = LRELEASE ${QMAKE_FILE_IN}
updateqm.variable_out = PRE_TARGETDEPS
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm

qm.files = $$TRANS_DIR1/*.qm
qm.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/qpacman/
qm.CONFIG += no_check_exist

desktop.files = QPacman.desktop
desktop.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/applications/

icon.files = pics/Pacman-arch_logo.png
icon.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/pixmaps/

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/

kde.files = QPacmanKDEService.desktop
greaterThan(QT_MAJOR_VERSION, 4): kde.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/kservices5/
lessThan(QT_MAJOR_VERSION, 5): kde.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/kde4/services/ServiceMenus/

INSTALLS += target qm desktop icon kde
