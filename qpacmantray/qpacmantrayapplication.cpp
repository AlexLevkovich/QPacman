#include "qpacmantrayapplication.h"
#include "libalpm.h"
#include "traypreferences.h"
#include <QTimer>

#define SHOW_PREFS_DELAY 3000

QPacmanTrayApplication::QPacmanTrayApplication(int &argc, char *argv[]) : SingleApplication(argc,argv,true) {
    m_mainWindow = NULL;
    m_wasTopMost = false;

    QApplication::setQuitOnLastWindowClosed(false);

    if (isSecondary()) {
        QMetaObject::invokeMethod(this,"secondary_init",Qt::QueuedConnection);
        return;
    }
    QMetaObject::invokeMethod(this,"primary_init",Qt::QueuedConnection);
}

QPacmanTrayApplication::~QPacmanTrayApplication() {
    if (m_mainWindow != NULL) delete m_mainWindow;
}

void QPacmanTrayApplication::secondary_init() {
    Alpm().askShowTrayOptions();
    QMetaObject::invokeMethod(this,"quit",Qt::QueuedConnection);
}

void QPacmanTrayApplication::primary_init() {
    m_mainWindow = new TrayPreferences(0);
    connect(m_mainWindow,SIGNAL(showRequest()),this,SLOT(putMainWindowOnTop()));
    QTimer::singleShot(SHOW_PREFS_DELAY,[&](){
        connect(Alpm::instance(),SIGNAL(show_tray_options()),this,SLOT(putMainWindowOnTop()));
    });
}

void QPacmanTrayApplication::putMainWindowOnTop() {
    if (m_mainWindow == NULL) return;
    m_mainWindow->setWindowFlags(m_mainWindow->windowFlags() | Qt::WindowStaysOnTopHint);
    m_wasTopMost = true;
    m_mainWindow->setVisible(true);
    m_mainWindow->activateWindow();
    m_mainWindow->raise();
}

bool QPacmanTrayApplication::notify(QObject *receiver, QEvent *event) {
    bool ret = SingleApplication::notify(receiver,event);
    if (m_wasTopMost && (receiver != NULL) && (event->type() == QEvent::ActivationChange) && qobject_cast<QMainWindow *>(receiver) && ((QMainWindow *)receiver)->isActiveWindow()) {
        QMainWindow * mainWindow = (QMainWindow *)receiver;
        mainWindow->setWindowFlags(mainWindow->windowFlags() & ~Qt::WindowStaysOnTopHint);
        mainWindow->setVisible(true);
        m_wasTopMost = false;
    }
    return ret;
}
