#-------------------------------------------------
#
# Project created by QtCreator 2019-01-29T09:24:01
#
#-------------------------------------------------

QT = core network
TARGET = qalpm
TEMPLATE = lib
VERSION = 2.0.0
DEFINES += QALPM_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

equals(USE_QDBUS, "ON") | isEmpty(USE_QDBUS) {
    QT += dbus
    DEFINES += USE_QDBUS
}

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/qpacman

ORG = AlexL
DEFINES += ORG=\\\"$$ORG\\\"

CONF=qalpm.conf
DEFINES += CONF=\\\"$$CONF\\\"

isEmpty(PACMANCONF) {
    PACMANCONF = /etc/pacman.conf
}
DEFINES += PACMANCONF=\\\"$$PACMANCONF\\\"

isEmpty(DBPATH) {
    DBPATH = /var/lib/pacman/
}
DEFINES += DBPATH=\\\"$$DBPATH\\\"

isEmpty(GPGDIR) {
    GPGDIR = /etc/pacman.d/gnupg/
}
DEFINES += GPGDIR=\\\"$$GPGDIR\\\"

isEmpty(LOGFILE) {
    LOGFILE = /var/log/pacman.log
}
DEFINES += LOGFILE=\\\"$$LOGFILE\\\"

isEmpty(CACHEDIR) {
    CACHEDIR = /var/cache/pacman/pkg/
}
DEFINES += CACHEDIR=\\\"$$CACHEDIR\\\"

HOOKDIR2 = /etc/pacman.d/hooks/
HOOKDIR1 = $$INSTALL_PREFIX/share/libalpm/hooks/
isEmpty(HOOKDIR) {
    DEFINES += HOOKDIR="\\\"$${HOOKDIR1}\ $${HOOKDIR2}\\\""
}

isEmpty(MIRRORLIST) {
    MIRRORLIST = /etc/pacman.d/mirrorlist
}
DEFINES += MIRRORLIST=\\\"$$MIRRORLIST\\\"

DBEXT = .db
DEFINES += DBEXT=\\\"$$DBEXT\\\"

FILESEXT = .files
DEFINES += FILESEXT=\\\"$$FILESEXT\\\"

LOGPREFIX = QALPM
DEFINES += LOGPREFIX=\\\"$$LOGPREFIX\\\"

SOURCES += \
    alpmconfig.cpp \
    alpmdb.cpp \
    alpmdownloader.cpp \
    alpmfuture.cpp \
    alpmpackage.cpp \
    archivefilesiterator.cpp \
    byteshumanizer.cpp \
    confreader.cpp \
    inotifier.cpp \
    libalpm.cpp \
    multidownloader.cpp \
    networkreplyproxy.cpp \
    simpledownloader.cpp

HEADERS += \
    alpmconfig.h \
    alpmdownloader.h \
    alpmfuture.h \
    alpmlist.h \
    archivefilesiterator.h \
    confreader.h \
    downloaderinterface.h \
    inotifier.h \
    libalpm.h \
    multidownloader.h \
    networkreplyproxy.h \
    alpmdb.h \
    alpmpackage.h \
    byteshumanizer.h \
    qalpmtypes.h \
    simpledownloader.h

LANGUAGES = ru be

defineReplace(prependAll) {
 for(a,$$1):result += $${2}_$${a}$$3
 return($$result)
}

TRANSLATIONS = $$prependAll(LANGUAGES, $$PWD/translations/lib$$TARGET, .ts)

LRELEASE_COMMAND =
LRELEASE_TARGET =
LRELEASE = $$[QT_INSTALL_BINS]/lrelease
for(tsfile, TRANSLATIONS) {
    qmfile = $$basename(tsfile)
    qmfile ~= s,.ts$,.qm,
    qmdir = $$OUT_PWD/translations
    qmfile = $$qmdir/$$qmfile
    !exists($$qmdir) {
        system($${QMAKE_MKDIR} \"$$qmdir\")
    }
    LRELEASE_TARGET += $$qmfile
    LRELEASE_COMMAND += $$LRELEASE $$tsfile -qm $$qmfile;
}

LUPDATE = $$[QT_INSTALL_BINS]/lupdate -locations relative -no-ui-lines -no-sort
updatets.target = TRANSLATIONS
updatets.commands = $$LUPDATE $$PWD/qalpm.pro
releasets.target = LRELEASE_TARGET
releasets.commands = $$LRELEASE_COMMAND
releasets.depends = updatets
PRE_TARGETDEPS += TRANSLATIONS
PRE_TARGETDEPS += LRELEASE_TARGET
QMAKE_EXTRA_TARGETS += updatets releasets

!exists("$$INSTALL_PREFIX/lib/libpam.so"): error("pam package needs to be installed!")
!exists("$$INSTALL_PREFIX/lib/libAppStreamQt.so"): error("appstream-qt and archlinux-appstream-data packages needs to be installed!")
!exists("$$INSTALL_PREFIX/lib/libbfd.so"): error("binutils package needs to be installed!")

CONFIG += link_pkgconfig
PKGCONFIG += libalpm libarchive
LIBS += -lcrypt -lpam -lAppStreamQt -lbfd

transinstall.files = $$prependAll(LANGUAGES, $$TRANS_DIR1/lib$$TARGET, .qm)
transinstall.CONFIG += no_check_exist
transinstall.path = $$INSTALL_ROOT/$$TRANS_DIR2

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/lib/
INSTALLS += target transinstall

