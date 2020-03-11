/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "packageprocessor.h"
#include <QAction>
#include <QToolBar>
#include <QStackedWidget>
#include <QPlainTextEdit>
#include "progressview.h"
#include "messagedialog.h"
#include "byteshumanizer.h"
#include "static.h"
#include "libalpm.h"
#include "themeicons.h"
#include <QMainWindow>
#include <QDebug>

PackageProcessor::PackageProcessor(ProgressView * view,QAction * cancelAction,OptionalDepsDlg * optdlg,QObject *parent) : PackageProcessorBase(parent) {
    eventItem = NULL;
    downloadItem = NULL;
    overalDownloadItem = NULL;
    installItem = NULL;
    overalInstallItem = NULL;
    removeItem = NULL;
    overalRemoveItem = NULL;
    overalHookItem = NULL;
    m_max_download_size = 0;
    m_downloaded_size = 0;
    m_xfered = 0;

    if (ThreadRun::isMethodExecuting()) ThreadRun::setTerminateFlag();

    m_optionalDepsDlg = optdlg;

    progressView = view;
    if (progressView == NULL) {
        qCritical() << "Programer error, in PackageProcessor";
        exit(145);
    }
    progressView->installEventFilter(this);
    if (cancelAction != NULL) connect(cancelAction,SIGNAL(triggered()),this,SLOT(cancelTriggered()));
    m_cancelAction = cancelAction;

    connect(Alpm::instance(),SIGNAL(information(const QString &)),this,SIGNAL(logString(const QString &)));
    connect(Alpm::instance(),SIGNAL(optdepends_event(const QString &,const StringStringMap &,const StringStringMap &)),this,SLOT(on_optdepends_event(const QString &,const StringStringMap &,const StringStringMap &)));
    connect(Alpm::instance(),SIGNAL(event(const QString &)),this,SLOT(on_event(const QString &)));
    connect(Alpm::instance(),SIGNAL(error(const QString &)),this,SLOT(on_error(const QString &)));
    connect(Alpm::instance(),SIGNAL(hook(const QString &,int,int)),this,SLOT(on_hook(const QString &,int,int)));
    connect(Alpm::instance(),SIGNAL(information(const QString &,bool)),this,SLOT(on_information(const QString &,bool)));
    connect(Alpm::instance(),SIGNAL(conflicts_progress(int)),this,SLOT(on_event_progress(int)));
    connect(Alpm::instance(),SIGNAL(diskspace_progress(int)),this,SLOT(on_event_progress(int)));
    connect(Alpm::instance(),SIGNAL(integrity_progress(int)),this,SLOT(on_event_progress(int)));
    connect(Alpm::instance(),SIGNAL(load_progress(int)),this,SLOT(on_event_progress(int)));
    connect(Alpm::instance(),SIGNAL(keyring_progress(int)),this,SLOT(on_event_progress(int)));
    connect(Alpm::instance(),SIGNAL(full_download_size_found(qint64)),this,SLOT(full_download_size_found(qint64)));
    connect(Alpm::instance(),SIGNAL(download_start(const QString &)),this,SLOT(download_start(const QString &)));
    connect(Alpm::instance(),SIGNAL(download_db_start(const QString &)),this,SLOT(download_db_start(const QString &)));
    connect(Alpm::instance(),SIGNAL(download_progress(const QString &,qint64,qint64)),this,SLOT(download_progress(const QString &,qint64,qint64)));
    connect(Alpm::instance(),SIGNAL(download_failed()),this,SIGNAL(canceled()));
    connect(Alpm::instance(),SIGNAL(download_completed()),this,SLOT(downloadok()));
    connect(Alpm::instance(),SIGNAL(install_progress(const QString &,int,size_t,size_t)),this,SLOT(install_progress(const QString &,int,size_t,size_t)));
    connect(Alpm::instance(),SIGNAL(remove_progress(const QString &,int,size_t,size_t)),this,SLOT(remove_progress(const QString &,int,size_t,size_t)));
    connect(progressView,SIGNAL(rowAdded(QStandardItem *,QStandardItem *)),this,SLOT(on_itemAdded(QStandardItem *,QStandardItem *)),Qt::QueuedConnection);
    if (m_optionalDepsDlg != NULL) connect(m_optionalDepsDlg,&QObject::destroyed,this,[&]() { m_optionalDepsDlg = NULL; });

    pkgChangeDlg.installEventFilter(this);
    pkgProviderDlg.installEventFilter(this);
    questionDlg.installEventFilter(this);

    if (m_cancelAction != NULL) m_cancelAction->setEnabled(false);
    QMetaObject::invokeMethod(this,"exec_process",Qt::QueuedConnection);
}

void PackageProcessor::exec_process() {
    if (ThreadRun::isMethodExecuting()) {
        QMetaObject::invokeMethod(this,"exec_process",Qt::QueuedConnection);
        return;
    }
    QMetaObject::invokeMethod(this,"completed",Qt::QueuedConnection,Q_ARG(ThreadRun::RC,process()));
    QMetaObject::invokeMethod(this,"deleteLater",Qt::QueuedConnection);
}

QMainWindow * PackageProcessor::createMainProcessorWindow(ProgressView ** view,QPlainTextEdit ** logView,QAction ** cancelAction,QAction ** logAction) {
    QMainWindow * mainWnd = new QMainWindow(NULL);
    mainWnd->setWindowIcon(QIcon("://pics/qpacman.svg"));
    QStackedWidget * mainWidget = new QStackedWidget(NULL);
    *view = new ProgressView(mainWidget,false);
    mainWidget->addWidget(*view);
    *logView = new QPlainTextEdit(mainWidget);
    mainWidget->addWidget(*logView);
    mainWnd->setCentralWidget(mainWidget);
    mainWidget->setCurrentIndex(0);
    *logAction = new QAction(mainWnd);
    (*logAction)->setEnabled(false);
    (*logAction)->setCheckable(true);
    (*logAction)->setChecked(false);
    (*logAction)->setIcon(ThemeIcons::get(ThemeIcons::LOG_VIEW));
    (*logAction)->setText(tr("Log"));
    (*logAction)->setToolTip(tr("Show the resulting log"));
    connect(*logAction,&QAction::triggered,[=](bool checked) { mainWidget->setCurrentIndex(checked?1:0); });
    *cancelAction = new QAction(mainWnd);
    (*cancelAction)->setEnabled(false);
    (*cancelAction)->setIcon(ThemeIcons::get(ThemeIcons::CANCEL));
    (*cancelAction)->setText(tr("Cancel"));
    (*cancelAction)->setToolTip(tr("Cancel the current operation"));
    (*cancelAction)->setShortcut(tr("Esc"));
    QToolBar * actionsToolBar = new QToolBar(mainWnd);
    actionsToolBar->setObjectName(QString::fromUtf8("actionsToolBar"));
    actionsToolBar->setMovable(false);
    actionsToolBar->setAllowedAreas(Qt::BottomToolBarArea);
    actionsToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    actionsToolBar->setFloatable(false);
    mainWnd->addToolBar(Qt::BottomToolBarArea, actionsToolBar);
    actionsToolBar->addAction(*logAction);
    actionsToolBar->addAction(*cancelAction);
    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    actionsToolBar->insertWidget(*cancelAction,empty);
    mainWnd->installEventFilter(new SaveStateFilter(mainWnd));
    mainWnd->setVisible(true);
    return mainWnd;
}

PackageProcessor::SaveStateFilter::SaveStateFilter(QMainWindow * wnd) : QObject(wnd) {
    m_wnd = wnd;
    is_shown = false;
}

bool PackageProcessor::SaveStateFilter::eventFilter(QObject *obj,QEvent *event) {
    if ((m_wnd != NULL) && (obj == m_wnd) && (event->type() == QEvent::Show) && !is_shown) {
        is_shown = true;
        m_wnd->restoreGeometry(QSettings().value("geometry/processingwindow").toByteArray());
    }
    if ((m_wnd != NULL) && (obj == m_wnd) && (event->type() == QEvent::Close)) {
        QSettings().setValue("geometry/processingwindow",m_wnd->saveGeometry());
    }
    return QObject::eventFilter(obj,event);
}

bool PackageProcessor::eventFilter(QObject *obj,QEvent *event) {
    if ((progressView != NULL) && (obj == progressView) && (event->type() == QEvent::Close)) {
        progressView->clear();
        i_currPkgName.clear();
        r_currPkgName.clear();
        currFileName.clear();
        m_max_download_size = 0;
        m_downloaded_size = 0;
        m_xfered = 0;
    }
    if ((obj == &pkgChangeDlg) && (event->type() == QEvent::Show)) {
        if (eventItem != NULL) eventItem->setMax();
    }
    return QObject::eventFilter(obj,event);
}

void PackageProcessor::on_event(const QString & str) {
    eventItem = progressView->appendEventProgressRow(str);
}

void PackageProcessor::on_error(const QString & str) {
    progressView->appendErrorRow(str);
}

void PackageProcessor::on_information(const QString & str,bool significant) {
    if (!significant) return;
    progressView->appendInformationRow(str);
}

void PackageProcessor::on_event_progress(int percents) {
    if (eventItem->maximum() != 100) eventItem->setRange(0,100);
    eventItem->setValue(percents);
}

void PackageProcessor::full_download_size_found(qint64 dl_size) {
    m_max_download_size = dl_size;
    m_downloaded_size = 0;
    overalDownloadItem = progressView->appendAverageDownloadProgressRow(tr("Overal download progress"),0,m_max_download_size);
}

void PackageProcessor::download_start(const QString & filename) {
    if (currFileName.isEmpty()) {
        if (m_cancelAction != NULL) m_cancelAction->setEnabled(true);
        if (overalDownloadItem == NULL) overalDownloadItem = progressView->appendAverageDownloadProgressRow(tr("Overal download progress"));
    }
    if (currFileName != filename) {
        downloadItem = progressView->appendDownloadProgressRow(filename+": "+tr("connecting..."));
        blockSignals(true);
        progressView->moveRowAtEnd(overalDownloadItem);
        blockSignals(false);
    }
}

void PackageProcessor::download_db_start(const QString & dbname) {
    if (m_cancelAction != NULL) m_cancelAction->setEnabled(true);
    downloadItem = progressView->appendDownloadProgressRow(dbname+": "+tr("updating..."));
}

void PackageProcessor::download_progress(const QString & filename,qint64 xfered,qint64 total) {
    if (currFileName != filename) {
        m_xfered = 0;
        downloadItem->setMessage(filename+((total > 0)?" ("+BytesHumanizer((double)total).toString()+")":""));
        currFileName = filename;
        downloadItem->setRange(0,total);
    }
    if (total != downloadItem->maximum()) {
        downloadItem->setMaximum(total);
        downloadItem->setMessage(filename+((total > 0)?" ("+BytesHumanizer((double)total).toString()+")":""));
    }
    if (downloadItem->maximum() > 0) downloadItem->setValue(xfered);
    m_downloaded_size += (xfered - m_xfered);
    m_xfered = xfered;
    if (overalDownloadItem != NULL && overalDownloadItem->maximum() > 0) overalDownloadItem->setValue(m_downloaded_size);
    if (overalDownloadItem != NULL && m_downloaded_size == m_max_download_size) {
        overalDownloadItem->setMax();
    }
}

void PackageProcessor::install_progress(const QString & pkg_name,int percent,size_t n_targets,size_t current_target) {
    if (overalInstallItem == NULL && n_targets > 0) overalInstallItem = progressView->appendAverageInstallProgressRow(tr("Overal installation progress"),0,100*n_targets,0);
    if (i_currPkgName != pkg_name) {
        i_currPkgName = pkg_name;
        installItem = progressView->appendInstallProgressRow(tr("Installing")+" "+pkg_name,0,100,0);
        blockSignals(true);
        progressView->moveRowAtEnd(overalInstallItem);
        blockSignals(false);
    }
    overalInstallItem->setValue(((current_target-1)*100)+percent);
    installItem->setValue(percent);
}

void PackageProcessor::remove_progress(const QString & pkg_name,int percent,size_t n_targets,size_t current_target) {
    if (overalRemoveItem == NULL && n_targets > 0) overalRemoveItem = progressView->appendAverageRemoveProgressRow(tr("Overal removal progress"),0,100*n_targets,0);
    if (r_currPkgName != pkg_name) {
        r_currPkgName = pkg_name;
        removeItem = progressView->appendRemoveProgressRow(tr("Removing")+" "+pkg_name,0,100,0);
        blockSignals(true);
        progressView->moveRowAtEnd(overalRemoveItem);
        blockSignals(false);
    }
    overalRemoveItem->setValue(((current_target-1)*100)+percent);
    removeItem->setValue(percent);
}

void PackageProcessor::cancelTriggered() {
    Alpm::instance()->setTerminateFlag();
    if (m_cancelAction != NULL) m_cancelAction->setEnabled(false);
}

void PackageProcessor::downloadok() {
    if (m_cancelAction != NULL) m_cancelAction->setEnabled(false);
    if (overalInstallItem != NULL) overalInstallItem->setMax();
}

void PackageProcessor::on_itemAdded(QStandardItem *,QStandardItem * item1) {
    if (item1->type() <= QStandardItem::UserType) return;
    if (item1->type() & (SimpleItem::Progress|SimpleItem::Average)) {
        if (item1->type() & (SimpleItem::Install|SimpleItem::Remove|SimpleItem::Download|SimpleItem::Hook)) return;
    }
    SimpleProgressItem * item = progressView->previousProgressItem(item1);
    if (item != NULL) item->setMax();
}

void PackageProcessor::on_hook(const QString & txt,int pos,int total) {
    if (overalHookItem == NULL && total > 0) overalHookItem = progressView->appendAverageHookProgressRow(tr("Overal hook progress"),0,total,0);
    if (overalHookItem != NULL && pos > 0) {
        progressView->appendHookRow(txt);
        blockSignals(true);
        progressView->moveRowAtEnd(overalHookItem);
        blockSignals(false);
        overalHookItem->setValue(pos);
    }
}

void PackageProcessor::on_optdepends_event(const QString & pkgname,const StringStringMap & installed,const StringStringMap & pending) {
    if (m_optionalDepsDlg != NULL) m_optionalDepsDlg->fill(pkgname,installed,pending);
}
