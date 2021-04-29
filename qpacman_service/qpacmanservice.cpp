/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "qpacmanservice.h"
#include "qpacmanservice_adaptor.h"
#include <QThread>
#include <QStringList>
#include <QNetworkProxy>
#include "alpmconfig.h"
#include "actionapplier.h"
#include <unistd.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <QDebug>

bool QPacmanService::m_files_executing = false;

const QString temporaryName() {
    QTemporaryFile tfile(QString(SYSTEMDCONFDIR)+QDir::separator()+SYSTEMDCONFFILEBASE"_XXXXXX.conf");
    if (!tfile.open()) return QString();
    return tfile.fileName();
}

QPacmanService::QPacmanService(QObject *parent) : QObject(parent) {
    new Alpm(this);

    qRegisterMetaType<String>("String");
    qDBusRegisterMetaType<String>();

    if (!Alpm::instance()->open(PACMANCONF)) {
        qCritical() << "Alpm Instance was not open!!!" << lastError();
        qApp->quit();
        return;
    }

    new QPacmanServiceAdaptor(this);
    QDBusConnection dbus = QDBusConnection::systemBus();
    if (!dbus.registerObject("/", this)) qDebug() << "Cannot register QPacmanService object!" << dbus.lastError();
    if (!dbus.registerService("com.alexl.qt.QPacmanService")) qDebug() << "Cannot register QPacmanService service!" << dbus.lastError();

    locked_outside = Alpm::instance()->isLocked();
    reload_is_needed = false;

    connect(Alpm::instance(),&Alpm::locking_changed,this,&QPacmanService::locking_changed);
    connect(Alpm::instance(),&Alpm::install_progress,this,&QPacmanService::install_progress);
    connect(Alpm::instance(),&Alpm::remove_progress,this,&QPacmanService::remove_progress);
    connect(Alpm::instance(),&Alpm::conflicts_progress,this,&QPacmanService::conflicts_progress);
    connect(Alpm::instance(),&Alpm::diskspace_progress,this,&QPacmanService::diskspace_progress);
    connect(Alpm::instance(),&Alpm::integrity_progress,this,&QPacmanService::integrity_progress);
    connect(Alpm::instance(),&Alpm::load_progress,this,&QPacmanService::load_progress);
    connect(Alpm::instance(),&Alpm::keyring_progress,this,&QPacmanService::keyring_progress);
    connect(Alpm::instance(),&Alpm::downloads_starting,this,&QPacmanService::downloads_starting);
    connect(Alpm::instance(),&Alpm::full_download_size_found,this,&QPacmanService::full_download_size_found);
    connect(Alpm::instance(),&Alpm::downloads_completed,this,&QPacmanService::downloads_completed);
    connect(Alpm::instance(),&Alpm::downloads_failed,this,&QPacmanService::downloads_failed);
    connect(Alpm::instance(),&Alpm::download_start,this,&QPacmanService::download_start);
    connect(Alpm::instance(),&Alpm::download_db_start,this,&QPacmanService::download_db_start);
    connect(Alpm::instance(),&Alpm::download_done,this,&QPacmanService::download_done);
    connect(Alpm::instance(),&Alpm::download_failed,this,&QPacmanService::download_failed);
    connect(Alpm::instance(),&Alpm::download_progress,this,&QPacmanService::download_progress);
    connect(Alpm::instance(),&Alpm::question,this,&QPacmanService::question);
    connect(Alpm::instance(),&Alpm::install_packages_confirmation,this,&QPacmanService::install_packages_confirmation);
    connect(Alpm::instance(),&Alpm::remove_packages_confirmation,this,&QPacmanService::remove_packages_confirmation);
    connect(Alpm::instance(),&Alpm::select_provider,this,&QPacmanService::select_provider);
    connect(Alpm::instance(),&Alpm::optdepends_event,this,&QPacmanService::optdepends_event);
    connect(Alpm::instance(),&Alpm::error,this,&QPacmanService::error);
    connect(Alpm::instance(),&Alpm::information,this,&QPacmanService::information);
    connect(Alpm::instance(),&Alpm::all_hooks,this,&QPacmanService::all_hooks);
    connect(Alpm::instance(),&Alpm::hook,this,&QPacmanService::hook);
    connect(Alpm::instance(),&Alpm::hook_completed,this,&QPacmanService::hook_completed);
    connect(Alpm::instance(),&Alpm::all_hooks_completed,this,&QPacmanService::all_hooks_completed);
    connect(Alpm::instance(),&Alpm::checking_file_conflicts,this,&QPacmanService::checking_file_conflicts);
    connect(Alpm::instance(),&Alpm::file_conflicts_checked,this,&QPacmanService::file_conflicts_checked);
    connect(Alpm::instance(),&Alpm::checking_pkg_deps,this,&QPacmanService::checking_pkg_deps);
    connect(Alpm::instance(),&Alpm::pkg_deps_checked,this,&QPacmanService::pkg_deps_checked);
    connect(Alpm::instance(),&Alpm::checking_internal_conflicts,this,&QPacmanService::checking_internal_conflicts);
    connect(Alpm::instance(),&Alpm::internal_conflicts_checked,this,&QPacmanService::internal_conflicts_checked);
    connect(Alpm::instance(),&Alpm::transaction_completed,this,&QPacmanService::transaction_completed);
    connect(Alpm::instance(),&Alpm::checking_integrity,this,&QPacmanService::checking_integrity);
    connect(Alpm::instance(),&Alpm::integrity_checked,this,&QPacmanService::integrity_checked);
    connect(Alpm::instance(),&Alpm::checking_diskspace,this,&QPacmanService::checking_diskspace);
    connect(Alpm::instance(),&Alpm::diskspace_checked,this,&QPacmanService::diskspace_checked);
    connect(Alpm::instance(),&Alpm::resolving_pkg_deps,this,&QPacmanService::resolving_pkg_deps);
    connect(Alpm::instance(),&Alpm::pkg_deps_resolved,this,&QPacmanService::pkg_deps_resolved);
    connect(Alpm::instance(),&Alpm::checking_keyring,this,&QPacmanService::checking_keyring);
    connect(Alpm::instance(),&Alpm::keyring_checked,this,&QPacmanService::keyring_checked);
    connect(Alpm::instance(),&Alpm::checking_key_download,this,&QPacmanService::checking_key_download);
    connect(Alpm::instance(),&Alpm::key_download_checked,this,&QPacmanService::key_download_checked);
    connect(Alpm::instance(),&Alpm::loading_pkg_files,this,&QPacmanService::loading_pkg_files);
    connect(Alpm::instance(),&Alpm::pkg_files_loaded,this,&QPacmanService::pkg_files_loaded);
    connect(Alpm::instance(),&Alpm::starting_scriplet,this,&QPacmanService::starting_scriplet);
    connect(Alpm::instance(),&Alpm::scriplet_executed,this,&QPacmanService::scriplet_executed);
    connect(Alpm::instance(),SIGNAL(method_finished(const QString&,const QStringList&,ThreadRun::RC)),this,SLOT(onmethod_finished(const QString&,const QStringList&,ThreadRun::RC)));
    connect(Alpm::instance(),SIGNAL(method_finished(const QString&,ThreadRun::RC)),this,SLOT(onmethod_finished(const QString&,ThreadRun::RC)));
    connect(Alpm::instance(),SIGNAL(method_finished(const QString&,const QList<AlpmPackage>&,ThreadRun::RC)),this,SLOT(onmethod_finished(const QString&,const QList<AlpmPackage>&,ThreadRun::RC)));
}

void QPacmanService::do_alpm_reopen() {
    if (!Alpm::instance()->reopen()) {
        qCritical() << "Alpm Instance was not reopen!!!" << lastError();
        qApp->quit();
        return;
    }
    QMetaObject::invokeMethod(this,"alpm_reopen",Qt::QueuedConnection);
}

void QPacmanService::locking_changed(const QString &,bool locked) {
    if (!locked && locked_outside) {
        reload_is_needed = true;
        if (!Alpm::instance()->isMethodExecuting()) {
            reload_is_needed = false;
            do_alpm_reopen();
        }
    }
    locked_outside = locked && !Alpm::instance()->isMethodExecuting();
}

void QPacmanService::onmethod_finished(const QString & name,ThreadRun::RC rc) {
    if (name == "Alpm::update_dbs") QMetaObject::invokeMethod(this,"alpm_reopen",Qt::QueuedConnection);

    remove_temp_file();

    emit method_finished(name,rc);
    if (reload_is_needed) {
        reload_is_needed = false;
        do_alpm_reopen();
    }
}

void QPacmanService::remove_temp_file() {
    if (!tempFileName.isEmpty()) {
        QFile(tempFileName).remove();
    }
    tempFileName.clear();
}

void QPacmanService::onmethod_finished(const QString & name,const QStringList & result,ThreadRun::RC rc) {
    emit method_finished(name,result,rc);
    if (reload_is_needed) {
        reload_is_needed = false;
        do_alpm_reopen();
    }
}

void QPacmanService::onmethod_finished(const QString & name,const QList<AlpmPackage> & result,ThreadRun::RC rc) {
    if (rc != ThreadRun::OK) {
        emit method_finished(name,rc);
        return;
    }

    QByteArray arr;
    for (const AlpmPackage & pkg: result) {
        arr.clear();
        QDataStream stream(&arr,QIODevice::WriteOnly);
        stream << pkg;
        emit package_queried(arr);
    }
    onmethod_finished(name,rc);
}

class MethodPauser {
public:
    MethodPauser() {
        if (Alpm::instance()->isMethodExecuting() && !Alpm::instance()->isMethodPaused()) Alpm::instance()->pauseMethodExecuting();
    }
    ~MethodPauser() {
        if (Alpm::instance()->isMethodExecuting() && Alpm::instance()->isMethodPaused()) Alpm::instance()->resumeMethodExecuting();
    }
};

QByteArray QPacmanService::packageUrl(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.url().toEncoded();
}

QString QPacmanService::packageFileName(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.fileName();
}

QString QPacmanService::packagePackager(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.packager();
}

QByteArray QPacmanService::packageArch(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return QByteArray(pkg.arch().latin1());
}

QStringList QPacmanService::packageLicenses(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.licenses();
}

QByteArray QPacmanService::createLocalPackage(const QString & pkgpath) {
    MethodPauser pauser;

    AlpmPackage pkg(pkgpath,false);
    QByteArray ret;
    QDataStream stream((QByteArray *)&ret,QIODevice::WriteOnly);
    stream << pkg;
    return ret;
}

void QPacmanService::deleteLocalPackage(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;

    pkg.setLocalDeleteFlag(true);
}

QStringList QPacmanService::packageRequiredby(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.requiredby();
}

QStringList QPacmanService::packageOptionalfor(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.optionalfor();
}

QByteArray QPacmanService::packageDepends(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    QByteArray ret;
    QDataStream stream2((QByteArray *)&ret,QIODevice::WriteOnly);
    stream2 << pkg.depends();
    return ret;
}

QByteArray QPacmanService::packageOptDepends(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    QByteArray ret;
    QDataStream stream2((QByteArray *)&ret,QIODevice::WriteOnly);
    stream2 << pkg.optdepends();
    return ret;
}

QByteArray QPacmanService::packageConflicts(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    QByteArray ret;
    QDataStream stream2((QByteArray *)&ret,QIODevice::WriteOnly);
    stream2 << pkg.conflicts();
    return ret;
}

QByteArray QPacmanService::packageProvides(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    QByteArray ret;
    QDataStream stream2((QByteArray *)&ret,QIODevice::WriteOnly);
    stream2 << pkg.provides();
    return ret;
}

QByteArray QPacmanService::packageReplaces(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    QByteArray ret;
    QDataStream stream2((QByteArray *)&ret,QIODevice::WriteOnly);
    stream2 << pkg.replaces();
    return ret;
}

QStringList QPacmanService::packageGroups(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.groups();
}

int QPacmanService::packageType(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return (int)pkg.type();
}

int QPacmanService::packageChangeStatus(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return (int)pkg.changeStatus();
}

qlonglong QPacmanService::packageSize(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return (qlonglong)pkg.size();
}

qlonglong QPacmanService::packageInstalledSize(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return (qlonglong)pkg.installedSize();
}

int QPacmanService::packageReason(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return (int)pkg.reason();
}

int QPacmanService::packageDefaultStatus(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return (int)pkg.defaultStatus();
}

bool QPacmanService::packageSetReason(const QByteArray & arr,int reason) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.setReason((AlpmPackage::Reason)reason);
}

QByteArray QPacmanService::packageIconUrl(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.iconUrl().toEncoded();
}

bool QPacmanService::packageSetChangeStatus(const QByteArray & arr,int status) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return AlpmPackage::setChangeStatus(pkg,(AlpmPackage::UserChangeStatus)status);
}

QByteArray QPacmanService::packagePossibleChangeStatuses(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    QByteArray ret;
    QDataStream stream2((QByteArray *)&ret,QIODevice::WriteOnly);
    stream2 << AlpmPackage::possibleChangeStatuses(pkg);
    return ret;
}

bool QPacmanService::packageIsOrphaned(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.isOrphaned();
}

qlonglong QPacmanService::packageBuildDate(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.buildDate().toSecsSinceEpoch();
}

qlonglong QPacmanService::packageInstallDate(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.installDate().toSecsSinceEpoch();
}

QString QPacmanService::executingMethodName() {
    MethodPauser pauser;

    return Alpm::instance()->executingMethodName();
}

void QPacmanService::answer(uint value) {
    Alpm::instance()->answer(value);
}

void QPacmanService::askShowTrayOptions() {
    emit show_tray_options();
}

void QPacmanService::setMethodTerminateFlag() {
    Alpm::instance()->setTerminateFlag();
}

bool QPacmanService::removeLockFile() {
    return Alpm::instance()->removeLockFile();
}

bool QPacmanService::cleanCacheDirs() {
    return Alpm::instance()->cleanCacheDirs();
}

QStringList QPacmanService::repos() {
    MethodPauser pauser;

    return Alpm::instance()->repos();
}

QStringList QPacmanService::groups() {
    MethodPauser pauser;

    return Alpm::instance()->groups();
}

QStringList QPacmanService::allDBs() {
    MethodPauser pauser;

    QStringList ret;
    ret << "local";
    for(AlpmDB & db: Alpm::instance()->allSyncDBs()) {
        ret << db.name();
    }
    return ret;
}

bool QPacmanService::queryPackages(const QString & name,int fieldType,int filter,const QString & group,const QString & repo) {
    return Alpm::instance()->queryPackages(name,(AlpmPackage::SearchFieldType)fieldType,(AlpmPackage::PackageFilter)filter,group,repo);
}

QString QPacmanService::lastError() {
    MethodPauser pauser;

    return Alpm::instance()->lastError();
}

class FilesMethodPauser: public MethodPauser {
public:
    FilesMethodPauser() : MethodPauser() {
        QPacmanService::m_files_executing = true;
    }
    ~FilesMethodPauser() {
        QPacmanService::m_files_executing = false;
    }
};

ThreadRun::RC QPacmanService::packageFiles(const QByteArray & arr) {
    if (m_files_executing) return ThreadRun::FORBIDDEN;
    FilesMethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;

    int i = 0;
    QList<AlpmPackage::FileInfo> files = pkg.files();
    for (AlpmPackage::FileInfo & file: files) {
        i++;
        QByteArray arr;
        QDataStream stream((QByteArray *)&arr,QIODevice::WriteOnly);
        stream << file;
        emit file_queried(arr,i,files.count());
    }
    if (files.count() <= 0) {
        QByteArray arr;
        QDataStream stream((QByteArray *)&arr,QIODevice::WriteOnly);
        stream << AlpmPackage::FileInfo();
        emit file_queried(arr,0,0);
    }
    return ThreadRun::OK;
}

int QPacmanService::pam_auth(int num_msg, const struct pam_message **,struct pam_response **resp, void *appdata_ptr) {
    struct pam_response *array_resp = (struct pam_response*) malloc(num_msg * sizeof(struct pam_response));
    QString pass = *(QString *)appdata_ptr;

    for (int i = 0; i < num_msg; i++) {
            array_resp[i].resp_retcode = 0;
            array_resp[i].resp = (char *)malloc(pass.toLocal8Bit().length() + 1);
            strcpy(array_resp[i].resp, pass.toLocal8Bit().constData());
    }

    *resp = array_resp;
    return PAM_SUCCESS;
}

bool QPacmanService::check_root_password(const QString & root_pw) {
    if (root_pw.isNull()) return false;

    pam_handle_t * pamh = NULL;
    pam_conv conv = {QPacmanService::pam_auth,(void *)&root_pw};
    int ret = pam_start("sudo","root",&conv,&pamh);
    if (ret != PAM_SUCCESS) return false;
    ret = pam_authenticate(pamh, 0);
    if (ret != PAM_SUCCESS) return false;
    ret = pam_acct_mgmt(pamh, 0);
    pam_end(pamh,ret);

    return (ret == PAM_SUCCESS);
}

ThreadRun::RC QPacmanService::installPackages(const String & root_pw,const QByteArray & pkgs,bool asdeps,const QByteArray & forcedpkgs) {
    if (!check_root_password(root_pw)) return ThreadRun::ROOTPW;
    QList<AlpmPackage> list1;
    QDataStream stream((QByteArray *)&pkgs,QIODevice::ReadOnly);
    stream >> list1;
    QList<AlpmPackage> list2;
    QDataStream stream2((QByteArray *)&forcedpkgs,QIODevice::ReadOnly);
    stream2 >> list2;

    return install_packages(list1,asdeps,list2);
}

ThreadRun::RC QPacmanService::install_packages(const QList<AlpmPackage> & pkgs,bool asdeps,const QList<AlpmPackage> & forcedpkgs) {
    QDir dir(SYSTEMDCONFDIR);
    dir.setNameFilters(QStringList() << QString(SYSTEMDCONFFILEBASE"_*.conf"));
    for (QString & fileName: dir.entryList()) {
        QFile(QString(SYSTEMDCONFDIR)+QDir::separator()+fileName).remove();
    }

    for (const AlpmPackage & pkg: pkgs) {
        if (pkg.name() == OWNPKGNAME) {
            tempFileName = temporaryName();
            if (!tempFileName.isEmpty()) {
                QFile(QString(SYSTEMDCONFDIR)+QDir::separator()+SYSTEMDCONFFILEBASE".conf").copy(tempFileName);
            }
            break;
        }
    }

    return Alpm::instance()->installPackages(pkgs,asdeps,forcedpkgs);
}

ThreadRun::RC QPacmanService::processPackages(const String & root_pw) {
    if (!check_root_password(root_pw)) return ThreadRun::ROOTPW;

    new ActionApplier(this);
    return ThreadRun::OK;
}

ThreadRun::RC QPacmanService::downloadPackages(const QByteArray & pkgs) {
    QList<AlpmPackage> list1;
    QDataStream stream((QByteArray *)&pkgs,QIODevice::ReadOnly);
    stream >> list1;
    return Alpm::instance()->downloadPackages(list1);
}

bool QPacmanService:: packageIsDownloaded(const QByteArray & arr) {
    MethodPauser pauser;

    AlpmPackage pkg;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> pkg;
    return pkg.isDownloaded(NULL);
}

ThreadRun::RC QPacmanService::updateDBs(bool force) {
    return Alpm::instance()->updateDBs(force);
}

uint QPacmanService::downloaderTimeout() {
    return AlpmConfig::downloaderTimeout();
}

uint QPacmanService::downloaderThreadCount() {
    return AlpmConfig::downloaderThreadCount();
}

QDataStream & operator<<(QDataStream &argument,const QNetworkProxy & proxy) {
    argument << (int)proxy.type();
    argument << proxy.hostName();
    argument << proxy.port();
    argument << proxy.user();
    argument << proxy.password();
    return argument;
}

const QDataStream & operator>>(const QDataStream &argument,QNetworkProxy & proxy) {
    int type;
    QString host;
    quint16 port;
    QString user;
    QString pw;
    (QDataStream &)argument >> type;
    (QDataStream &)argument >> host;
    (QDataStream &)argument >> port;
    (QDataStream &)argument >> user;
    (QDataStream &)argument >> pw;
    proxy.setType((QNetworkProxy::ProxyType)type);
    proxy.setHostName(host);
    proxy.setPort(port);
    proxy.setUser(user);
    proxy.setPassword(pw);
    return argument;
}

QByteArray QPacmanService::downloaderProxy() {
    QByteArray ret;
    QDataStream stream((QByteArray *)&ret,QIODevice::WriteOnly);
    stream << AlpmConfig::downloaderProxy();
    return ret;
}

bool QPacmanService::useSystemIcons() {
    return AlpmConfig::useSystemIcons();
}

void QPacmanService::setDownloaderTimeout(uint value) {
    AlpmConfig::setDownloaderTimeout(value);
}

void QPacmanService::setDownloaderThreads(uint value) {
    AlpmConfig::setDownloaderThreads(value);
}

void QPacmanService::setDownloaderProxy(const QByteArray & arr) {
    QNetworkProxy proxy;
    QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
    stream >> proxy;
    AlpmConfig::setDownloaderProxy(proxy);
}

void QPacmanService::setUsingSystemIcons(bool flag) {
    AlpmConfig::setUsingSystemIcons(flag);
}

QByteArray QPacmanService::updates() {
    QByteArray ret;
    QDataStream stream((QByteArray *)&ret,QIODevice::WriteOnly);
    stream << Alpm::instance()->updates();
    return ret;
}

bool QPacmanService::areMarkedPackages() {
    return (AlpmPackage::changedStatusPackages().count() > 0);
}

QString QPacmanService::dbExtension() {
    return AlpmConfig::dbExtension();
}

QStringList QPacmanService::dbExtensions() {
    return AlpmConfig::dbExtensions();
}

bool QPacmanService::setDBExtension(const QString & dbext) {
    return AlpmConfig::setDBExtension(dbext);
}

QString QPacmanService::rootDir() {
    return Alpm::config()->rootDir();
}

QString QPacmanService::dbPath() {
    return Alpm::config()->dbPath();
}

QString QPacmanService::gpgDir() {
    return Alpm::config()->gpgDir();
}

QString QPacmanService::logFileName() {
    return Alpm::config()->logFileName();
}

QString QPacmanService::arch() {
    return Alpm::config()->arch();
}

bool QPacmanService::doUseSysLog() {
    return Alpm::config()->doUseSysLog();
}

bool QPacmanService::doDisableDownloadTimeout() {
    return Alpm::config()->doDisableDownloadTimeout();
}

QStringList QPacmanService::sigLevel() {
    return Alpm::config()->sigLevel();
}

QStringList QPacmanService::localFileSigLevel() {
    return Alpm::config()->localFileSigLevel();
}

QStringList QPacmanService::remoteFileSigLevel() {
    return Alpm::config()->remoteFileSigLevel();
}

QStringList QPacmanService::holdPkgs() {
    return Alpm::config()->holdPkgs();
}

QStringList QPacmanService::cacheDirs() {
    return Alpm::config()->cacheDirs();
}

QStringList QPacmanService::syncFirstPkgs() {
    return Alpm::config()->syncFirstPkgs();
}

QStringList QPacmanService::hookDirs() {
    return Alpm::config()->hookDirs();
}

QStringList QPacmanService::ignoreGroups() {
    return Alpm::config()->ignoreGroups();
}

QStringList QPacmanService::ignorePkgs() {
    return Alpm::config()->ignorePkgs();
}

QStringList QPacmanService::noExtractPkgs() {
    return Alpm::config()->noExtractPkgs();
}

QStringList QPacmanService::noUpgradePkgs() {
    return Alpm::config()->noUpgradePkgs();
}

QByteArray QPacmanService::findByPackageName(const QString & pkgname) {
    MethodPauser pauser;

    QByteArray ret;
    QDataStream stream((QByteArray *)&ret,QIODevice::WriteOnly);
    stream << Alpm::instance()->findByPackageName(pkgname);
    return ret;
}

QByteArray QPacmanService::findLocalPackage(const QString & pkgname) {
    MethodPauser pauser;

    QByteArray ret;
    QDataStream stream((QByteArray *)&ret,QIODevice::WriteOnly);
    stream << Alpm::instance()->localDB().findByPackageName(pkgname);
    return ret;
}

QByteArray QPacmanService::findLocalPackage(const QString & name,const QString & version) {
    MethodPauser pauser;

    QByteArray ret;
    QDataStream stream((QByteArray *)&ret,QIODevice::WriteOnly);
    stream << Alpm::instance()->localDB().findByPackageNameVersion(name,version);
    return ret;
}

QByteArray QPacmanService::findByPackageNameProvides(const QByteArray & provide) {
    MethodPauser pauser;

    AlpmPackage::Dependence dep;
    QDataStream stream2((QByteArray *)&provide,QIODevice::ReadOnly);
    stream2 >> dep;

    QByteArray ret;
    QDataStream stream((QByteArray *)&ret,QIODevice::WriteOnly);
    stream << Alpm::instance()->findByPackageNameProvides(dep);
    return ret;
}

QByteArray QPacmanService::findLocalByPackageNameProvides(const QByteArray & provide) {
    MethodPauser pauser;

    AlpmPackage::Dependence dep;
    QDataStream stream2((QByteArray *)&provide,QIODevice::ReadOnly);
    stream2 >> dep;

    QByteArray ret;
    QDataStream stream((QByteArray *)&ret,QIODevice::WriteOnly);
    stream << Alpm::instance()->localDB().findByPackageNameProvides(dep);
    return ret;
}

void QPacmanService::resetPackageChangeStatuses() {
    MethodPauser pauser;

    AlpmPackage::resetAllChangeStatuses();
}
