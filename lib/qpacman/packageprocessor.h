/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef PACKAGEPROCESSOR_H
#define PACKAGEPROCESSOR_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include "packagechangesdialog.h"
#include "packageprovidersdialog.h"
#include "questiondialog.h"
#include "optionaldepsdlg.h"

class ProgressView;
class QMainWindow;
class QAction;
class SimpleProgressItem;
class QStandardItem;
class QPlainTextEdit;
class QStackedWidget;

class PackageProcessorBase : public QObject {
    Q_OBJECT
public:
    PackageProcessorBase(QObject * parent = NULL) : QObject(parent) {}
signals:
    void logString(const QString & str);
    void completed(ThreadRun::RC ok,const QString & error);
};

class PackageProcessor : public PackageProcessorBase {
    Q_OBJECT
public:
    PackageProcessor(ProgressView * view = NULL,QAction * cancelAction = NULL,OptionalDepsDlg * optdlg = NULL,QObject *parent = nullptr);
    static QMainWindow * createMainProcessorWindow(ProgressView ** view,QPlainTextEdit ** logView,QAction ** cancelAction,QAction ** logAction);

protected:
    class SaveStateFilter : public QObject {
    public:
        SaveStateFilter(QMainWindow * wnd);
    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;
    private:
        QMainWindow * m_wnd;
        bool is_shown;
    };

    bool eventFilter(QObject *obj,QEvent *event);

private slots:
    void on_error(const QString & str);
    void on_information(const QString & str,bool significant);
    void on_event(int id,const QString & str);
    void on_optdepends_event(const QString & pkgname,const StringStringMap & installed,const StringStringMap & pending);
    void on_event_progress(int percents);
    void on_hook(const QString & txt,int pos,int total);
    void download_start(const QString & filename);
    void download_db_start(const QString & dbname);
    void download_progress(const QString & filename,qint64 xfered,qint64 total);
    void full_download_size_found(qint64 dl_size);
    void install_progress(const QString & pkg_name,int percent,int n_targets,int current_target);
    void remove_progress(const QString & pkg_name,int percent,int n_targets,int current_target);
    void cancelTriggered();
    void downloadok();
    void on_itemAdded(const QModelIndex & item0,const QModelIndex & item1);
    void exec_process();
    void on_event_completed(int id);
    void on_method_finished(const QString & name,const QStringList & result,ThreadRun::RC rc);
    void on_method_finished(const QString & name,ThreadRun::RC rc);
    void on_timeout();

protected slots:
    virtual ThreadRun::RC process(const QString & pw) = 0;

private:
    ProgressView * progressView;

    QString i_currPkgName;
    QString r_currPkgName;
    QString currFileName;
    qint64 m_max_download_size;
    qint64 m_downloaded_size;
    qint64 m_xfered;
    QAction * m_cancelAction;
    PackageChangesDialog   pkgChangeDlg;
    PackageProvidersDialog pkgProviderDlg;
    OptionalDepsDlg      * m_optionalDepsDlg;
    QuestionDialog         questionDlg;
    SimpleProgressItem * eventItem;
    SimpleProgressItem * overalHookItem;
    SimpleProgressItem * downloadItem;
    SimpleProgressItem * overalDownloadItem;
    SimpleProgressItem * installItem;
    SimpleProgressItem * overalInstallItem;
    SimpleProgressItem * removeItem;
    SimpleProgressItem * overalRemoveItem;
    QMap<int,SimpleProgressItem *> eventItems;
    QTimer m_start_timer;

    static QString m_pw;
    static int root_counter;

    friend class ActionApplier;
};

#endif // PACKAGEPROCESSOR_H
