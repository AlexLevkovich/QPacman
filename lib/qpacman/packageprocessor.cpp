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
#include "windowcenterer.h"
#include "packagechangesdialog.h"
#include "packageprovidersdialog.h"
#include "questiondialog.h"
#include "optionaldepsdlg.h"
#include "rootdialog.h"
#include <QMainWindow>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

int PackageProcessor::root_counter = 0;
QString PackageProcessor::m_pw;

PackageProcessor::PackageProcessor(ProgressView * view,QAction * cancelAction,OptionalDepsDlg * optdlg,QObject *parent) : PackageProcessorBase(parent) {
    eventItem = nullptr;
    downloadItem = nullptr;
    overalDownloadItem = nullptr;
    installItem = nullptr;
    overalInstallItem = nullptr;
    removeItem = nullptr;
    overalRemoveItem = nullptr;
    overalHookItem = nullptr;
    m_max_download_size = 0;
    m_downloaded_size = 0;
    m_xfered = 0;

    pkgChangeDlg = new PackageChangesDialog();
    pkgProviderDlg = new PackageProvidersDialog();
    questionDlg = new QuestionDialog();

    qApp->installEventFilter(this);
    if (!Alpm::instance()->executingMethodName().isEmpty()) Alpm::instance()->setMethodTerminateFlag();

    m_optionalDepsDlg = optdlg;

    progressView = view;
    if (progressView != nullptr) progressView->installEventFilter(this);
    if (cancelAction != nullptr) connect(cancelAction,SIGNAL(triggered()),this,SLOT(cancelTriggered()));
    m_cancelAction = cancelAction;

    connect(pkgChangeDlg,&QObject::destroyed,this,[&]() { pkgChangeDlg = nullptr; });
    connect(pkgProviderDlg,&QObject::destroyed,this,[&]() { pkgProviderDlg = nullptr; });
    connect(questionDlg,&QObject::destroyed,this,[&]() { questionDlg = nullptr; });
    connect(Alpm::instance(),&Alpm::information,this,&PackageProcessor::logString);
    connect(Alpm::instance(),&Alpm::optdepends_event,this,&PackageProcessor::on_optdepends_event);
    connect(Alpm::instance(),&Alpm::all_hooks,this,[&](const QString & message) { on_event(1,message); });
    connect(Alpm::instance(),&Alpm::checking_pkg_deps,this,[&](const QString & message) { on_event(2,message); });
    connect(Alpm::instance(),&Alpm::checking_file_conflicts,this,[&](const QString & message) { on_event(3,message); });
    connect(Alpm::instance(),&Alpm::resolving_pkg_deps,this,[&](const QString & message) { on_event(4,message); });
    connect(Alpm::instance(),&Alpm::checking_internal_conflicts,this,[&](const QString & message) { on_event(5,message); });
    connect(Alpm::instance(),&Alpm::checking_integrity,this,[&](const QString & message) { on_event(6,message); });
    connect(Alpm::instance(),&Alpm::checking_diskspace,this,[&](const QString & message) { on_event(7,message); });
    connect(Alpm::instance(),&Alpm::checking_keyring,this,[&](const QString & message) { on_event(8,message); });
    connect(Alpm::instance(),&Alpm::checking_key_download,this,[&](const QString & message) { on_event(9,message); });
    connect(Alpm::instance(),&Alpm::loading_pkg_files,this,[&](const QString & message) { on_event(10,message); });
    connect(Alpm::instance(),&Alpm::starting_scriplet,this,[&](const QString & message) { on_event(11,message); });
    connect(Alpm::instance(),&Alpm::all_hooks_completed,this,[&]() { on_event_completed(1); });
    connect(Alpm::instance(),&Alpm::pkg_deps_checked,this,[&]() { on_event_completed(2); });
    connect(Alpm::instance(),&Alpm::file_conflicts_checked,this,[&]() { on_event_completed(3); });
    connect(Alpm::instance(),&Alpm::pkg_deps_resolved,this,[&]() { on_event_completed(4); });
    connect(Alpm::instance(),&Alpm::internal_conflicts_checked,this,[&]() { on_event_completed(5); });
    connect(Alpm::instance(),&Alpm::integrity_checked,this,[&]() { on_event_completed(6); });
    connect(Alpm::instance(),&Alpm::diskspace_checked,this,[&]() { on_event_completed(7); });
    connect(Alpm::instance(),&Alpm::keyring_checked,this,[&]() { on_event_completed(8); });
    connect(Alpm::instance(),&Alpm::key_download_checked,this,[&]() { on_event_completed(9); });
    connect(Alpm::instance(),&Alpm::pkg_files_loaded,this,[&]() { on_event_completed(10); });
    connect(Alpm::instance(),&Alpm::scriplet_executed,this,[&]() { on_event_completed(11); });
    connect(Alpm::instance(),&Alpm::error,this,&PackageProcessor::on_error);
    connect(Alpm::instance(),&Alpm::hook,this,&PackageProcessor::on_hook);
    connect(Alpm::instance(),&Alpm::information,this,&PackageProcessor::on_information);
    connect(Alpm::instance(),&Alpm::conflicts_progress,this,&PackageProcessor::on_event_progress);
    connect(Alpm::instance(),&Alpm::diskspace_progress,this,&PackageProcessor::on_event_progress);
    connect(Alpm::instance(),&Alpm::integrity_progress,this,&PackageProcessor::on_event_progress);
    connect(Alpm::instance(),&Alpm::load_progress,this,&PackageProcessor::on_event_progress);
    connect(Alpm::instance(),&Alpm::keyring_progress,this,&PackageProcessor::on_event_progress);
    connect(Alpm::instance(),&Alpm::full_download_size_found,this,&PackageProcessor::full_download_size_found);
    connect(Alpm::instance(),&Alpm::download_start,this,&PackageProcessor::download_start);
    connect(Alpm::instance(),&Alpm::download_db_start,this,&PackageProcessor::download_db_start);
    connect(Alpm::instance(),&Alpm::download_progress,this,&PackageProcessor::download_progress);
    connect(Alpm::instance(),&Alpm::downloads_completed,this,&PackageProcessor::downloadok);
    connect(Alpm::instance(),&Alpm::install_progress,this,&PackageProcessor::install_progress);
    connect(Alpm::instance(),&Alpm::remove_progress,this,&PackageProcessor::remove_progress);
    connect(Alpm::instance(),SIGNAL(method_finished(QString,QStringList,ThreadRun::RC)),this,SLOT(on_method_finished(QString,QStringList,ThreadRun::RC)));
    connect(Alpm::instance(),SIGNAL(method_finished(QString,ThreadRun::RC)),this,SLOT(on_method_finished(QString,ThreadRun::RC)));
    connect(&m_start_timer,&QTimer::timeout,this,&PackageProcessor::on_timeout);
    if (progressView != nullptr) connect(progressView,&ProgressView::rowAdded,this,&PackageProcessor::on_itemAdded,Qt::QueuedConnection);
    if (m_optionalDepsDlg != nullptr) connect(m_optionalDepsDlg,&QObject::destroyed,this,[&]() { m_optionalDepsDlg = nullptr; });

    if (m_cancelAction != nullptr) m_cancelAction->setEnabled(false);
    QMetaObject::invokeMethod(this,"exec_process",Qt::QueuedConnection);
    m_start_timer.start(3000);
}

PackageProcessor::~PackageProcessor() {
    if (pkgChangeDlg != nullptr) delete pkgChangeDlg;
    if (pkgProviderDlg != nullptr) delete pkgProviderDlg;
    if (questionDlg != nullptr) delete questionDlg;
}

void PackageProcessor::on_timeout() {
    m_start_timer.stop();
    QString err = tr("DBUS server is busy, may be another application is using it!");
    emit logString(err);
    if (progressView != nullptr) progressView->appendErrorRow(err);
    QMetaObject::invokeMethod(this,"completed",Qt::QueuedConnection,Q_ARG(ThreadRun::RC,ThreadRun::FORBIDDEN),Q_ARG(QString,err));
    deleteLater();
}

void PackageProcessor::on_method_finished(const QString &,const QStringList &,ThreadRun::RC rc) {
    QMetaObject::invokeMethod(this,"completed",Qt::QueuedConnection,Q_ARG(ThreadRun::RC,rc),Q_ARG(QString,Alpm::instance()->lastError()));
    deleteLater();
}

void PackageProcessor::on_method_finished(const QString &,ThreadRun::RC rc) {
    QMetaObject::invokeMethod(this,"completed",Qt::QueuedConnection,Q_ARG(ThreadRun::RC,rc),Q_ARG(QString,Alpm::instance()->lastError()));
    deleteLater();
}

void PackageProcessor::exec_process() {
    if (!Alpm::instance()->executingMethodName().isEmpty()) {
        QMetaObject::invokeMethod(this,"exec_process",Qt::QueuedConnection);
        return;
    }
    m_start_timer.stop();
    switch (process(m_pw)) {
    case ThreadRun::BAD:
    case ThreadRun::TERMINATED:
    case ThreadRun::FORBIDDEN:
        break;
    case ThreadRun::ROOTPW:
    {
        if (root_counter < 3) {
            RootDialog dlg;
            if (dlg.exec() == QDialog::Accepted) {
                m_pw = dlg.password();
                QMetaObject::invokeMethod(this,"exec_process",Qt::QueuedConnection);
            }
            else {
                QString err = tr("Superuser authentication failed!");
                emit logString(err);
                if (progressView != nullptr) progressView->appendErrorRow(err);
                QMetaObject::invokeMethod(this,"completed",Qt::QueuedConnection,Q_ARG(ThreadRun::RC,ThreadRun::BAD),Q_ARG(QString,err));
                deleteLater();
            }
            root_counter++;
        }
        else {
            QString err = tr("Superuser authentication failed: too many attempts!");
            emit logString(err);
            if (progressView != nullptr) progressView->appendErrorRow(err);
            QMetaObject::invokeMethod(this,"completed",Qt::QueuedConnection,Q_ARG(ThreadRun::RC,ThreadRun::BAD),Q_ARG(QString,err));
            deleteLater();
            root_counter = 0;
        }
        break;
    }
    case ThreadRun::OK:
        root_counter = 0;
        break;
    }
}

QMainWindow * PackageProcessor::createMainProcessorWindow(ProgressView ** view,QPlainTextEdit ** logView,QAction ** cancelAction,QAction ** logAction,const QString & tittle) {
    QMainWindow * mainWnd = new QMainWindow(nullptr);
    mainWnd->setWindowIcon(QIcon("://pics/qpacman.svg"));
    QStackedWidget * mainWidget = new QStackedWidget(mainWnd);
    *view = new ProgressView(mainWidget,false);
    mainWidget->addWidget(*view);
    *logView = new QPlainTextEdit(mainWidget);
    mainWidget->addWidget(*logView);
    mainWnd->setCentralWidget(mainWidget);
    mainWidget->setCurrentIndex(0);
    QToolBar * actionsToolBar = new QToolBar(mainWnd);
    actionsToolBar->setObjectName(QString::fromUtf8("actionsToolBar"));
    actionsToolBar->setMovable(false);
    actionsToolBar->setAllowedAreas(Qt::BottomToolBarArea);
    actionsToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    actionsToolBar->setFloatable(false);
    mainWnd->addToolBar(Qt::BottomToolBarArea, actionsToolBar);
    *cancelAction = new QAction(actionsToolBar);
    (*cancelAction)->setEnabled(false);
    (*cancelAction)->setIcon(ThemeIcons::get(ThemeIcons::CANCEL));
    (*cancelAction)->setText(tr("Cancel"));
    (*cancelAction)->setToolTip(tr("Cancel the current operation"));
    (*cancelAction)->setShortcut(tr("Esc"));
    *logAction = new QAction(actionsToolBar);
    (*logAction)->setEnabled(false);
    (*logAction)->setCheckable(true);
    (*logAction)->setChecked(false);
    (*logAction)->setIcon(ThemeIcons::get(ThemeIcons::LOG_VIEW));
    (*logAction)->setText(tr("Log"));
    (*logAction)->setToolTip(tr("Show the resulting log"));
    connect(*logAction,&QAction::triggered,mainWidget,[=](bool flag) {mainWidget->setCurrentIndex(flag?1:0);});
    actionsToolBar->addAction(*logAction);
    actionsToolBar->addAction(*cancelAction);
    QWidget* empty = new QWidget(actionsToolBar);
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    actionsToolBar->insertWidget(*cancelAction,empty);
    mainWnd->installEventFilter(new SaveStateFilter(mainWnd));
    QRect rect = QApplication::desktop()->screenGeometry(mainWnd);
    mainWnd->resize((rect.width()*2)/3,(rect.height()*2)/3);
    mainWnd->setAttribute(Qt::WA_DeleteOnClose);
    mainWnd->setWindowTitle(tittle);
    mainWnd->setVisible(true);
    return mainWnd;
}

PackageProcessor::SaveStateFilter::SaveStateFilter(QMainWindow * wnd) : QObject(wnd) {
    m_wnd = wnd;
    is_shown = false;
}

bool PackageProcessor::SaveStateFilter::eventFilter(QObject *obj,QEvent *event) {
    if ((m_wnd != nullptr) && (obj == m_wnd) && (event->type() == QEvent::Show) && !is_shown) {
        is_shown = true;
        m_wnd->restoreGeometry(QSettings().value("geometry/processingwindow").toByteArray());
        new WindowCenterer(m_wnd);
    }
    if ((m_wnd != nullptr) && (obj == m_wnd) && (event->type() == QEvent::Close)) {
        QSettings().setValue("geometry/processingwindow",m_wnd->saveGeometry());
    }
    return QObject::eventFilter(obj,event);
}

bool PackageProcessor::eventFilter(QObject *obj,QEvent *event) {
    if ((progressView != nullptr) && (obj == progressView) && (event->type() == QEvent::Close)) {
        progressView->clear();
        i_currPkgName.clear();
        r_currPkgName.clear();
        currFileName.clear();
        m_max_download_size = 0;
        m_downloaded_size = 0;
        m_xfered = 0;
        eventItem = nullptr;
        eventItems.clear();
    }

    if ((event->type() == QEvent::Close) && obj->inherits("QWidgetWindow") && !Alpm::instance()->executingMethodName().isEmpty() && (obj->parent() == nullptr)) return true;

    return QObject::eventFilter(obj,event);
}

void PackageProcessor::on_event(int id,const QString & str) {
    if (progressView == nullptr) return;
    eventItem = progressView->appendEventProgressRow(str);
    eventItems.insert(id,eventItem);
}

void PackageProcessor::on_event_completed(int id) {
    SimpleProgressItem * eventItem = eventItems.contains(id)?eventItems[id]:nullptr;
    if (eventItem != nullptr) eventItem->setMax();
}

void PackageProcessor::on_error(const QString & err) {
    QString str = err;
    if (progressView == nullptr) return;
    if (str.length() > 100) str = str.mid(0,100);
    progressView->appendErrorRow(str);
}

void PackageProcessor::on_information(const QString & str,bool significant) {
    if (progressView == nullptr) return;
    if (!significant) return;
    progressView->appendInformationRow(str);
}

void PackageProcessor::on_event_progress(int percents) {
    if (eventItem->maximum() != 100) eventItem->setRange(0,100);
    eventItem->setValue(percents);
}

void PackageProcessor::full_download_size_found(qint64 dl_size) {
    if (progressView == nullptr) return;
    m_max_download_size = dl_size;
    m_downloaded_size = 0;
    overalDownloadItem = progressView->appendAverageDownloadProgressRow(tr("Overal download progress"),0,m_max_download_size);
}

void PackageProcessor::download_start(const QString & filename) {
    if (progressView == nullptr) return;
    if (currFileName.isEmpty()) {
        if (m_cancelAction != nullptr) m_cancelAction->setEnabled(true);
        if (overalDownloadItem == nullptr) overalDownloadItem = progressView->appendAverageDownloadProgressRow(tr("Overal download progress"));
    }
    if (currFileName != filename) {
        downloadItem = progressView->appendDownloadProgressRow(filename+": "+tr("connecting..."));
        blockSignals(true);
        progressView->moveRowAtEnd(overalDownloadItem);
        blockSignals(false);
    }
}

void PackageProcessor::download_db_start(const QString & dbname) {
    if (progressView == nullptr) return;
    if (m_cancelAction != nullptr) m_cancelAction->setEnabled(true);
    downloadItem = progressView->appendDownloadProgressRow(dbname+": "+tr("updating..."));
}

void PackageProcessor::download_progress(const QString & filename,qint64 xfered,qint64 total) {
    if (progressView == nullptr) return;
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
    if (overalDownloadItem != nullptr && overalDownloadItem->maximum() > 0) overalDownloadItem->setValue(m_downloaded_size);
    if (overalDownloadItem != nullptr && m_downloaded_size == m_max_download_size) {
        overalDownloadItem->setMax();
    }
}

void PackageProcessor::install_progress(const QString & pkg_name,int percent,int n_targets,int current_target) {
    if (progressView == nullptr) return;
    if (overalInstallItem == nullptr && n_targets > 0) overalInstallItem = progressView->appendAverageInstallProgressRow(tr("Overal installation progress"),0,100*n_targets,0);
    if (i_currPkgName != pkg_name) {
        i_currPkgName = pkg_name;
        installItem = progressView->appendInstallProgressRow(tr("Installing")+" "+pkg_name,0,100,0);
        blockSignals(true);
        progressView->moveRowAtEnd(overalInstallItem);
        blockSignals(false);
    }
    if (overalInstallItem != nullptr) overalInstallItem->setValue(((current_target-1)*100)+percent);
    installItem->setValue(percent);
}

void PackageProcessor::remove_progress(const QString & pkg_name,int percent,int n_targets,int current_target) {
    if (progressView == nullptr) return;
    if (overalRemoveItem == nullptr && n_targets > 0) overalRemoveItem = progressView->appendAverageRemoveProgressRow(tr("Overal removal progress"),0,100*n_targets,0);
    if (r_currPkgName != pkg_name) {
        r_currPkgName = pkg_name;
        removeItem = progressView->appendRemoveProgressRow(tr("Removing")+" "+pkg_name,0,100,0);
        blockSignals(true);
        progressView->moveRowAtEnd(overalRemoveItem);
        blockSignals(false);
    }
    if (overalRemoveItem != nullptr) overalRemoveItem->setValue(((current_target-1)*100)+percent);
    removeItem->setValue(percent);
}

void PackageProcessor::cancelTriggered() {
    Alpm::instance()->setMethodTerminateFlag();
    if (m_cancelAction != nullptr) m_cancelAction->setEnabled(false);
}

void PackageProcessor::downloadok() {
    if (m_cancelAction != nullptr) m_cancelAction->setEnabled(false);
    if (overalInstallItem != nullptr) overalInstallItem->setMax();
}

void PackageProcessor::on_itemAdded(const QModelIndex &,const QModelIndex & index) {
    if (progressView == nullptr) return;
    QStandardItem * item1 = ((QStandardItemModel *)progressView->model())->itemFromIndex(index);
    if (item1->type() <= QStandardItem::UserType) return;
    if (item1->type() & (SimpleItem::Progress|SimpleItem::Average)) {
        if (item1->type() & (SimpleItem::Install|SimpleItem::Remove|SimpleItem::Download|SimpleItem::Hook)) return;
    }
    SimpleProgressItem * item = progressView->previousProgressItem(item1);
    if (item != nullptr) item->setMax();
}

void PackageProcessor::on_hook(const QString & txt,int pos,int total) {
    if (progressView == nullptr) return;
    if (overalHookItem == nullptr && total > 0) overalHookItem = progressView->appendAverageHookProgressRow(tr("Overal hook progress"),0,total,0);
    if (overalHookItem != nullptr && pos > 0) {
        progressView->appendHookRow(txt);
        blockSignals(true);
        progressView->moveRowAtEnd(overalHookItem);
        blockSignals(false);
        overalHookItem->setValue(pos);
    }
}

void PackageProcessor::on_optdepends_event(const QString & pkgname,const StringStringMap & installed,const StringStringMap & pending) {
    if (m_optionalDepsDlg != nullptr) m_optionalDepsDlg->fill(pkgname,installed,pending);
}
