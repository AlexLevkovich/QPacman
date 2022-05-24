/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "libalpm.h"
#include "qalpmtypes.h"
#include <QEventLoop>
#include <QTimer>
#include "qpacmanservice_interface.h"
#include "dbusstring.h"

Alpm * m_alpm_instance = nullptr;

QDBusArgument & operator<<(QDBusArgument &argument,const ThreadRun::RC & rc) {
    argument.beginStructure();
    argument << (int)rc;
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument,ThreadRun::RC & rc) {
    argument.beginStructure();
    int val;
    argument >> val;
    rc = (ThreadRun::RC)val;
    argument.endStructure();

    return argument;
}

Alpm::Alpm(QObject * parent) : QObject(parent) {
    m_valid = false;
    m_interface = nullptr;

    if (m_alpm_instance == nullptr) m_alpm_instance = this;
    else {
        m_error = tr("The only one instance of Alpm is allowed!!!");
        QMetaObject::invokeMethod(this,"error",Qt::QueuedConnection,Q_ARG(QString,m_error));
        return;
    }

    qRegisterMetaType<ThreadRun::RC>("ThreadRun::RC");
    qDBusRegisterMetaType<ThreadRun::RC>();
    qDBusRegisterMetaType<StringStringMap>();
    qRegisterMetaType<String>("String");
    qDBusRegisterMetaType<String>();
    m_interface = new ComAlexlQtQPacmanServiceInterface(ComAlexlQtQPacmanServiceInterface::staticInterfaceName(),"/",QDBusConnection::systemBus(),this);
    m_interface->connection().interface()->startService(ComAlexlQtQPacmanServiceInterface::staticInterfaceName());

    m_valid = true;
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::install_progress,this,&Alpm::install_progress);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::remove_progress,this,&Alpm::remove_progress);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::conflicts_progress,this,&Alpm::conflicts_progress);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::diskspace_progress,this,&Alpm::diskspace_progress);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::integrity_progress,this,&Alpm::integrity_progress);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::load_progress,this,&Alpm::load_progress);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::keyring_progress,this,&Alpm::keyring_progress);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::downloads_starting,this,&Alpm::downloads_starting);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::full_download_size_found,this,&Alpm::full_download_size_found);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::downloads_completed,this,&Alpm::downloads_completed);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::downloads_failed,this,&Alpm::downloads_failed);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::download_start,this,&Alpm::download_start);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::download_db_start,this,&Alpm::download_db_start);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::download_done,this,&Alpm::download_done);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::download_failed,this,&Alpm::download_failed);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::download_progress,this,&Alpm::download_progress);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::question,this,&Alpm::question);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::install_packages_confirmation,this,&Alpm::install_packages_confirmation);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::remove_packages_confirmation,this,&Alpm::remove_packages_confirmation);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::select_provider,this,&Alpm::select_provider);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::optdepends_event,this,&Alpm::optdepends_event);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::error,this,&Alpm::error);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::information,this,&Alpm::information);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::all_hooks,this,&Alpm::all_hooks);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::hook,this,&Alpm::hook);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::hook_completed,this,&Alpm::hook_completed);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::all_hooks_completed,this,&Alpm::all_hooks_completed);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::checking_file_conflicts,this,&Alpm::checking_file_conflicts);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::file_conflicts_checked,this,&Alpm::file_conflicts_checked);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::checking_pkg_deps,this,&Alpm::checking_pkg_deps);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::pkg_deps_checked,this,&Alpm::pkg_deps_checked);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::checking_internal_conflicts,this,&Alpm::checking_internal_conflicts);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::internal_conflicts_checked,this,&Alpm::internal_conflicts_checked);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::transaction_completed,this,&Alpm::transaction_completed);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::checking_integrity,this,&Alpm::checking_integrity);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::integrity_checked,this,&Alpm::integrity_checked);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::checking_diskspace,this,&Alpm::checking_diskspace);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::diskspace_checked,this,&Alpm::diskspace_checked);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::resolving_pkg_deps,this,&Alpm::resolving_pkg_deps);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::pkg_deps_resolved,this,&Alpm::pkg_deps_resolved);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::checking_keyring,this,&Alpm::checking_keyring);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::keyring_checked,this,&Alpm::keyring_checked);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::checking_key_download,this,&Alpm::checking_key_download);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::key_download_checked,this,&Alpm::key_download_checked);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::loading_pkg_files,this,&Alpm::loading_pkg_files);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::pkg_files_loaded,this,&Alpm::pkg_files_loaded);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::starting_scriplet,this,&Alpm::starting_scriplet);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::scriplet_executed,this,&Alpm::scriplet_executed);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::alpm_reopen,this,&Alpm::alpm_reopen);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::do_start_dbrefresher,this,&Alpm::do_start_dbrefresher);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::dbs_update_started,this,&Alpm::dbs_update_started);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::show_tray_options,this,&Alpm::show_tray_options);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::package_updater_started,this,&Alpm::package_updater_started);
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::do_start_package_updater,this,&Alpm::do_start_package_updater);
    connect(m_interface,SIGNAL(method_finished(QString,QStringList,ThreadRun::RC)),this,SIGNAL(method_finished(QString,QStringList,ThreadRun::RC)));
    connect(m_interface,SIGNAL(method_finished(QString,ThreadRun::RC)),this,SIGNAL(method_finished(QString,ThreadRun::RC)));
    connect(m_interface,&ComAlexlQtQPacmanServiceInterface::package_queried,this,&Alpm::onpackage_queried);
}

Alpm::~Alpm() {
    m_alpm_instance = nullptr;
}

Alpm * Alpm::instance() {
    return m_alpm_instance;
}

bool Alpm::isValid() const {
    return m_interface != nullptr && m_valid;
}

void Alpm::onpackage_queried(const QByteArray &result) {
    QDataStream stream((QByteArray *)&result,QIODevice::ReadOnly);
    AlpmPackage pkg;
    stream >> pkg;
    emit package_queried(pkg);
}

QList<AlpmPackage::FileInfo> Alpm::packageFiles(const AlpmPackage & pkg) {
    if (!isValid()) return QList<AlpmPackage::FileInfo>();

    while (isSignalConnected(QMetaMethod::fromSignal(&ComAlexlQtQPacmanServiceInterface::file_queried))) {
        QCoreApplication::processEvents();
    }

    QList<AlpmPackage::FileInfo> ret;
    QEventLoop loop;
    QMetaObject::Connection conn = connect(m_interface,&ComAlexlQtQPacmanServiceInterface::file_queried,this,[&](const QByteArray & arr,int index,int count) {
        AlpmPackage::FileInfo path;
        QDataStream stream((QByteArray *)&arr,QIODevice::ReadOnly);
        stream >> path;
        if (index == 0 && count == 0) {
            loop.quit();
            return;
        }
        ret.append(path);
        if (index >= count) loop.quit();
    });

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QTimer::singleShot(0,[&](){
        ThreadRun::RC rc;
        if (replyToValue<ThreadRun::RC>(m_interface->packageFiles(arr),rc)) {
            if (rc != ThreadRun::OK) loop.quit();
        }
        else loop.quit();
    });
    loop.exec();
    disconnect(conn);

    return ret;
}

bool Alpm::queryPackages(const QString & name,AlpmPackage::SearchFieldType fieldType,AlpmPackage::PackageFilter filter,const QString & group,const QString & repo) {
    bool ret = false;
    if (!isValid()) return false;
    if (replyToValue<bool>(m_interface->queryPackages(name,fieldType,filter,group,repo),ret)) return ret;
    return false;
}

QString Alpm::lastError() const {
    if (!m_error.isEmpty()) return m_error;

    QString ret;
    if (replyToValue<QString>(m_interface->lastError(),ret)) return ret;

    return m_error;
}

QUrl Alpm::packageUrl(const AlpmPackage & pkg) const {
    if (!isValid()) return QUrl();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QByteArray ret;
    if (replyToValue<QByteArray>(m_interface->packageUrl(arr),ret)) return QUrl::fromEncoded(ret);
    return QUrl();
}

QString Alpm::packageFileName(const AlpmPackage & pkg) const {
    if (!isValid()) return QString();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QString ret;
    if (replyToValue<QString>(m_interface->packageFileName(arr),ret)) return ret;
    return QString();
}

QString Alpm::packagePackager(const AlpmPackage & pkg) const {
    if (!isValid()) return QString();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QString ret;
    if (replyToValue<QString>(m_interface->packagePackager(arr),ret)) return ret;
    return QString();
}

QByteArray Alpm::packageArch(const AlpmPackage & pkg) const {
    if (!isValid()) return QByteArray();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QByteArray ret;
    if (replyToValue<QByteArray>(m_interface->packageArch(arr),ret)) return ret;
    return QByteArray();
}

QStringList Alpm::packageLicenses(const AlpmPackage & pkg) const {
    if (!isValid()) return QStringList();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QStringList ret;
    if (replyToValue<QStringList>(m_interface->packageLicenses(arr),ret)) return ret;
    return QStringList();
}

QStringList Alpm::packageGroups(const AlpmPackage & pkg) const {
    if (!isValid()) return QStringList();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QStringList ret;
    if (replyToValue<QStringList>(m_interface->packageGroups(arr),ret)) return ret;
    return QStringList();
}

AlpmPackage::Type Alpm::packageType(const AlpmPackage & pkg) const {
    if (!isValid()) return AlpmPackage::Unknown;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    int ret;
    if (replyToValue<int>(m_interface->packageType(arr),ret)) return (AlpmPackage::Type)ret;
    return AlpmPackage::Unknown;
}

AlpmPackage::UserChangeStatus Alpm::packageChangeStatus(const AlpmPackage & pkg) const {
    if (!isValid()) return AlpmPackage::DO_NOTHING;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    int ret;
    if (replyToValue<int>(m_interface->packageChangeStatus(arr),ret)) return (AlpmPackage::UserChangeStatus)ret;
    return AlpmPackage::DO_NOTHING;
}

qint64 Alpm::packageSize(const AlpmPackage & pkg) const {
    if (!isValid()) return -1;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    qlonglong ret;
    if (replyToValue<qlonglong>(m_interface->packageSize(arr),ret)) return (qint64)ret;
    return -1;
}

qint64 Alpm::packageInstalledSize(const AlpmPackage & pkg) const {
    if (!isValid()) return -1;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    qlonglong ret;
    if (replyToValue<qlonglong>(m_interface->packageInstalledSize(arr),ret)) return (qint64)ret;
    return -1;
}

AlpmPackage::Reason Alpm::packageReason(const AlpmPackage & pkg) const {
    if (!isValid()) return AlpmPackage::Undefined;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    int ret;
    if (replyToValue<int>(m_interface->packageReason(arr),ret)) return (AlpmPackage::Reason)ret;
    return AlpmPackage::Undefined;
}

bool Alpm::packageSetReason(const AlpmPackage & pkg,AlpmPackage::Reason reason) const {
    if (!isValid()) return false;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    bool ret;
    if (replyToValue<bool>(m_interface->packageSetReason(arr,(int)reason),ret)) return ret;
    return false;
}

bool Alpm::packageSetChangeStatus(const AlpmPackage & pkg,AlpmPackage::UserChangeStatus status) const {
    if (!isValid()) return false;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    bool ret;
    if (replyToValue<bool>(m_interface->packageSetChangeStatus(arr,(int)status),ret)) return ret;
    return false;
}

QList<AlpmPackage::UserChangeStatus> Alpm::packagePossibleChangeStatuses(const AlpmPackage & pkg) {
    if (!isValid()) return QList<AlpmPackage::UserChangeStatus>();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->packagePossibleChangeStatuses(arr),ret)) return QList<AlpmPackage::UserChangeStatus>();
    QDataStream stream2(&ret,QIODevice::ReadOnly);
    QList<AlpmPackage::UserChangeStatus> list;
    stream2 >> list;
    return list;
}

AlpmPackage::UserChangeStatus Alpm::packageDefaultStatus(const AlpmPackage & pkg) const {
    if (!isValid()) return AlpmPackage::DO_NOTHING;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    int ret;
    if (replyToValue<int>(m_interface->packageDefaultStatus(arr),ret)) return (AlpmPackage::UserChangeStatus)ret;
    return AlpmPackage::DO_NOTHING;
}

QUrl Alpm::packageIconUrl(const AlpmPackage & pkg) const {
    if (!isValid()) return QUrl();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QByteArray ret;
    if (replyToValue<QByteArray>(m_interface->packageIconUrl(arr),ret)) return QUrl::fromEncoded(ret);
    return QUrl();
}

bool Alpm::packageIsOrphaned(const AlpmPackage & pkg) const {
    if (!isValid()) return false;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    bool ret;
    if (replyToValue<bool>(m_interface->packageIsOrphaned(arr),ret)) return ret;
    return false;
}

QList<AlpmPackage::Dependence> Alpm::packageDepends(const AlpmPackage & pkg) const {
    if (!isValid()) return QList<AlpmPackage::Dependence>();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->packageDepends(arr),ret)) return QList<AlpmPackage::Dependence>();
    QDataStream stream2(&ret,QIODevice::ReadOnly);
    QList<AlpmPackage::Dependence> list;
    stream2 >> list;
    return list;
}

QList<AlpmPackage::Dependence> Alpm::packageOptDepends(const AlpmPackage & pkg) const {
    if (!isValid()) return QList<AlpmPackage::Dependence>();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->packageOptDepends(arr),ret)) return QList<AlpmPackage::Dependence>();
    QDataStream stream2(&ret,QIODevice::ReadOnly);
    QList<AlpmPackage::Dependence> list;
    stream2 >> list;
    return list;
}

QList<AlpmPackage::Dependence> Alpm::packageConflicts(const AlpmPackage & pkg) const {
    if (!isValid()) return QList<AlpmPackage::Dependence>();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->packageConflicts(arr),ret)) return QList<AlpmPackage::Dependence>();
    QDataStream stream2(&ret,QIODevice::ReadOnly);
    QList<AlpmPackage::Dependence> list;
    stream2 >> list;
    return list;
}

QList<AlpmPackage::Dependence> Alpm::packageProvides(const AlpmPackage & pkg) const {
    if (!isValid()) return QList<AlpmPackage::Dependence>();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->packageProvides(arr),ret)) return QList<AlpmPackage::Dependence>();
    QDataStream stream2(&ret,QIODevice::ReadOnly);
    QList<AlpmPackage::Dependence> list;
    stream2 >> list;
    return list;
}

QList<AlpmPackage::Dependence> Alpm::packageReplaces(const AlpmPackage & pkg) const {
    if (!isValid()) return QList<AlpmPackage::Dependence>();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->packageReplaces(arr),ret)) return QList<AlpmPackage::Dependence>();
    QDataStream stream2(&ret,QIODevice::ReadOnly);
    QList<AlpmPackage::Dependence> list;
    stream2 >> list;
    return list;
}

QDateTime Alpm::packageBuildDate(const AlpmPackage & pkg) const {
    if (!isValid()) return QDateTime();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    qlonglong ret;
    if (replyToValue<qlonglong>(m_interface->packageBuildDate(arr),ret)) return QDateTime::fromSecsSinceEpoch(ret);
    return QDateTime();
}

QDateTime Alpm::packageInstallDate(const AlpmPackage & pkg) const {
    if (!isValid()) return QDateTime();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;
    qlonglong ret;
    if (replyToValue<qlonglong>(m_interface->packageInstallDate(arr),ret)) return QDateTime::fromSecsSinceEpoch(ret);
    return QDateTime();
}

ThreadRun::RC Alpm::installPackages(const QString & root_pw,const QList<AlpmPackage> & pkgs,bool asdeps,const QList<AlpmPackage> & forcedpkgs) {
    if (!isValid()) return ThreadRun::BAD;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkgs;
    QByteArray arr2;
    QDataStream stream2(&arr2,QIODevice::WriteOnly);
    stream2 << forcedpkgs;
    ThreadRun::RC ret;
    if (replyToValue<ThreadRun::RC>(m_interface->installPackages(root_pw,arr,asdeps,arr2),ret)) return ret;
    return ThreadRun::BAD;
}

ThreadRun::RC Alpm::processPackages(const QString & root_pw) {
    if (!isValid()) return ThreadRun::BAD;

    ThreadRun::RC ret;
    if (replyToValue<ThreadRun::RC>(m_interface->processPackages(root_pw),ret)) return ret;
    return ThreadRun::BAD;
}

ThreadRun::RC Alpm::downloadPackages(const QList<AlpmPackage> & pkgs) {
    if (!isValid()) return ThreadRun::BAD;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkgs;
    ThreadRun::RC ret;
    if (replyToValue<ThreadRun::RC>(m_interface->downloadPackages(arr),ret)) return ret;
    return ThreadRun::BAD;
}

ThreadRun::RC Alpm::updateDBs(bool force) {
    if (!isValid()) return ThreadRun::BAD;

    ThreadRun::RC ret;
    if (replyToValue<ThreadRun::RC>(m_interface->updateDBs(force),ret)) return ret;
    return ThreadRun::BAD;
}

bool Alpm::answer(uint flag) {
    if (!isValid()) return false;

    replyToVoid(m_interface->answer(flag));

    return true;
}

void Alpm::setMethodTerminateFlag() {
    if (!isValid()) return;

    replyToVoid(m_interface->setMethodTerminateFlag());
}

QString Alpm::executingMethodName() const {
    if (!isValid()) return QString();

    QString ret;
    if (replyToValue<QString>(m_interface->executingMethodName(),ret)) return ret;
    return ret;
}

bool Alpm::askShowTrayOptions() {
    if (!isValid()) return false;

    replyToVoid(m_interface->askShowTrayOptions());

    return true;
}

bool Alpm::removeLockFile() {
    if (!isValid()) return false;

    bool ret;
    if (replyToValue<bool>(m_interface->removeLockFile(),ret)) return ret;
    return false;
}

bool Alpm::cleanCacheDirs() {
    if (!isValid()) return false;

    bool ret;
    if (replyToValue<bool>(m_interface->cleanCacheDirs(),ret)) return ret;
    return false;
}

Alpm::Repo::SigLevel::SigLevel() {
    m_object = Both;
    m_check = Default;
    m_allowed = Nothing;
}

Alpm::Repo::SigLevel::SigLevel(SigObject object,SigCheck check,SigAllowed allowed) {
    m_object = object;
    m_check = check;
    m_allowed = allowed;
}

bool Alpm::Repo::SigLevel::operator==(const SigLevel & other) const {
    return (m_object == other.m_object && m_check == other.m_check && m_allowed == other.m_allowed);
}

QDataStream & operator<<(QDataStream &argument,const Alpm::Repo::SigLevel & level) {
    argument << (uint)level.object();
    argument << (uint)level.check();
    argument << (uint)level.allowed();

    return argument;
}

const QDataStream & operator>>(const QDataStream &argument,Alpm::Repo::SigLevel & level) {
    uint val;
    (QDataStream &)argument >> val;
    level.m_object = (Alpm::Repo::SigObject)val;
    (QDataStream &)argument >> val;
    level.m_check = (Alpm::Repo::SigCheck)val;
    (QDataStream &)argument >> val;
    level.m_allowed = (Alpm::Repo::SigAllowed)val;

    return argument;
}

Alpm::Repo::Usage::Usage() {
    m_sync = true;
    m_search = true;
    m_install = true;
    m_upgrade = true;
}

Alpm::Repo::Usage::Usage(bool sync,bool search,bool install,bool upgrade) {
    m_sync = sync;
    m_search = search;
    m_install = install;
    m_upgrade = upgrade;
}

bool Alpm::Repo::Usage::isAll() const {
    return ((m_sync && m_search && m_install && m_upgrade) || (!m_sync && !m_search && !m_install && !m_upgrade));
}

QDataStream & operator<<(QDataStream &argument,const Alpm::Repo::Usage & usage) {
    argument << (int)usage.isSync();
    argument << (int)usage.isSearch();
    argument << (int)usage.isInstall();
    argument << (int)usage.isUpgrade();

    return argument;
}

const QDataStream & operator>>(const QDataStream &argument,Alpm::Repo::Usage & usage) {
    int val;
    (QDataStream &)argument >> val;
    usage.m_sync = (bool)val;
    (QDataStream &)argument >> val;
    usage.m_search = (bool)val;
    (QDataStream &)argument >> val;
    usage.m_install = (bool)val;
    (QDataStream &)argument >> val;
    usage.m_upgrade = (bool)val;

    return argument;
}

bool Alpm::Repo::isValid() const {
    return m_valid;
}

Alpm::Repo::Repo(const QString & name,const QStringList & urls, const QStringList & arches,const ListSigLevel & siglevels,const Usage & usages) {
    m_valid = true;
    m_name = name.toLower();
    if (m_name == "local") {
        m_valid = false;
        return;
    }

    m_arches = arches;
    m_servers = urls;
    for (QString & server: m_servers) {
        for (const QString & arch: arches) server = server.replace("$repo",m_name).replace("$arch",arch);
    }
    m_servers.removeDuplicates();

    m_usage = usages;
    m_siglevel = siglevels;
}

Alpm::Repo::Repo(const Alpm::Repo & repo) {
    *this = repo;
}

Alpm::Repo::Repo() {
    m_valid = false;
    m_usage = Usage();
}

Alpm::Repo & Alpm::Repo::operator=(const Alpm::Repo &repo) {
    m_valid = repo.m_valid;
    m_usage = repo.m_usage;
    m_name = repo.m_name;
    m_arches = repo.m_arches;
    m_servers = repo.m_servers;
    m_siglevel = repo.m_siglevel;
    return *this;
}

QDataStream & operator<<(QDataStream &argument,const Alpm::Repo & repo) {
    argument << repo.usage();
    argument << repo.name();
    argument << repo.arches();
    argument << repo.servers();
    argument << repo.siglevel();
    argument << (bool)repo.m_valid;

    return argument;
}

const QDataStream & operator>>(const QDataStream &argument,Alpm::Repo & repo) {
    (QDataStream &)argument >> repo.m_usage;
    (QDataStream &)argument >> repo.m_name;
    (QDataStream &)argument >> repo.m_arches;
    (QDataStream &)argument >> repo.m_servers;
    (QDataStream &)argument >> repo.m_siglevel;
    (QDataStream &)argument >> repo.m_valid;

    return argument;
}

QList<Alpm::Repo> Alpm::repos() const {
    if (!isValid()) return QList<Alpm::Repo>();

    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->repos(),ret)) return QList<Alpm::Repo>();
    QDataStream stream(&ret,QIODevice::ReadOnly);
    QList<Alpm::Repo> list;
    stream >> list;
    return list;
}

QStringList Alpm::groups() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (replyToValue<QStringList>(m_interface->groups(),ret)) return ret;
    return ret;
}

QStringList Alpm::allDBs() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (replyToValue<QStringList>(m_interface->allDBs(),ret)) return ret;
    return ret;
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

uint Alpm::downloaderTimeout() const {
    if (!isValid()) return 0;

    uint ret;
    if (replyToValue<uint>(m_interface->downloaderTimeout(),ret)) return ret;
    return 0;
}

uint Alpm::downloaderThreadCount() const {
    if (!isValid()) return 0;

    uint ret;
    if (replyToValue<uint>(m_interface->downloaderThreadCount(),ret)) return ret;
    return 0;
}

QNetworkProxy Alpm::downloaderProxy() const {
    if (!isValid()) return QNetworkProxy();

    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->downloaderProxy(),ret)) return QNetworkProxy();
    QDataStream stream(&ret,QIODevice::ReadOnly);
    QNetworkProxy proxy;
    stream >> proxy;
    return proxy;
}

bool Alpm::useSystemIcons() const {
    if (!isValid()) return false;

    bool ret;
    if (replyToValue<bool>(m_interface->useSystemIcons(),ret)) return ret;
    return false;
}

void Alpm::setDownloaderTimeout(uint value) {
    if (!isValid()) return;

    replyToVoid(m_interface->setDownloaderTimeout(value));
}

void Alpm::setDownloaderThreads(uint value) {
    if (!isValid()) return;

    replyToVoid(m_interface->setDownloaderThreads(value));
}

void Alpm::setDownloaderProxy(const QNetworkProxy & proxy) {
    if (!isValid()) return;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << proxy;
    replyToVoid(m_interface->setDownloaderProxy(arr));
}

void Alpm::setUsingSystemIcons(bool flag) {
    if (!isValid()) return;

    replyToVoid(m_interface->setUsingSystemIcons(flag));
}

QList<AlpmPackage> Alpm::updates() const {
    if (!isValid()) return QList<AlpmPackage>();

    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->updates(),ret)) return QList<AlpmPackage>();
    QDataStream stream(&ret,QIODevice::ReadOnly);
    QList<AlpmPackage> list;
    stream >> list;
    return list;
}

bool Alpm::areMarkedPackages() const {
    if (!isValid()) return false;

    bool ret;
    if (!replyToValue<bool>(m_interface->areMarkedPackages(),ret)) return false;
    return ret;
}

QString Alpm::dbExtension() const {
    if (!isValid()) return QString();

    QString ret;
    if (!replyToValue<QString>(m_interface->dbExtension(),ret)) return QString();
    return ret;
}

QStringList Alpm::dbExtensions() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->dbExtensions(),ret)) return QStringList();
    return ret;
}

bool Alpm::setDBExtension(const QString & dbext) {
    if (!isValid()) return false;

    bool ret;
    if (!replyToValue<bool>(m_interface->setDBExtension(dbext),ret)) return false;
    return ret;
}

QString Alpm::rootDir() const {
    if (!isValid()) return QString();

    QString ret;
    if (!replyToValue<QString>(m_interface->rootDir(),ret)) return QString();
    return ret;
}

QString Alpm::dbPath() const {
    if (!isValid()) return QString();

    QString ret;
    if (!replyToValue<QString>(m_interface->dbPath(),ret)) return QString();
    return ret;
}

QString Alpm::gpgDir() const {
    if (!isValid()) return QString();

    QString ret;
    if (!replyToValue<QString>(m_interface->gpgDir(),ret)) return QString();
    return ret;
}

QString Alpm::logFileName() const {
    if (!isValid()) return QString();

    QString ret;
    if (!replyToValue<QString>(m_interface->logFileName(),ret)) return QString();
    return ret;
}

QStringList Alpm::arches() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->arches(),ret)) return QStringList();
    return ret;
}

bool Alpm::doUseSysLog() const {
    if (!isValid()) return false;

    bool ret;
    if (!replyToValue<bool>(m_interface->doUseSysLog(),ret)) return false;
    return ret;
}

bool Alpm::doDisableDownloadTimeout() const {
    if (!isValid()) return false;

    bool ret;
    if (!replyToValue<bool>(m_interface->doDisableDownloadTimeout(),ret)) return false;
    return ret;
}

QStringList Alpm::sigLevel() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->sigLevel(),ret)) return QStringList();
    return ret;
}

QStringList Alpm::localFileSigLevel() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->localFileSigLevel(),ret)) return QStringList();
    return ret;
}

QStringList Alpm::remoteFileSigLevel() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->remoteFileSigLevel(),ret)) return QStringList();
    return ret;
}

QStringList Alpm::holdPkgs() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->holdPkgs(),ret)) return QStringList();
    return ret;
}

QStringList Alpm::cacheDirs() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->cacheDirs(),ret)) return QStringList();
    return ret;
}

QStringList Alpm::syncFirstPkgs() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->syncFirstPkgs(),ret)) return QStringList();
    return ret;
}

QStringList Alpm::hookDirs() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->hookDirs(),ret)) return QStringList();
    return ret;
}

QStringList Alpm::ignoreGroups() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->ignoreGroups(),ret)) return QStringList();
    return ret;
}

QStringList Alpm::ignorePkgs() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->ignorePkgs(),ret)) return QStringList();
    return ret;
}

QStringList Alpm::noExtractPkgs() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->noExtractPkgs(),ret)) return QStringList();
    return ret;
}

QStringList Alpm::noUpgradePkgs() const {
    if (!isValid()) return QStringList();

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->noUpgradePkgs(),ret)) return QStringList();
    return ret;
}

bool Alpm::packageIsDownloaded(const AlpmPackage & pkg) const {
    if (!isValid()) return false;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;

    bool ret;
    if (!replyToValue<bool>(m_interface->packageIsDownloaded(arr),ret)) return false;
    return ret;
}

QStringList Alpm::packageRequiredby(const AlpmPackage & pkg) {
    if (!isValid()) return QStringList();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->packageRequiredby(arr),ret)) return QStringList();
    return ret;
}

QStringList Alpm::packageOptionalfor(const AlpmPackage & pkg) {
    if (!isValid()) return QStringList();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;

    QStringList ret;
    if (!replyToValue<QStringList>(m_interface->packageOptionalfor(arr),ret)) return QStringList();
    return ret;
}

QList<AlpmPackage> Alpm::findByPackageName(const QString & pkgname) const {
    if (!isValid()) return QList<AlpmPackage>();

    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->findByPackageName(pkgname),ret)) return QList<AlpmPackage>();

    QList<AlpmPackage> list;
    QDataStream stream(&ret,QIODevice::ReadOnly);
    stream >> list;

    return list;
}

AlpmPackage Alpm::findLocalPackage(const QString & pkgname) const {
    if (!isValid()) return AlpmPackage();

    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->findLocalPackage(pkgname),ret)) return AlpmPackage();

    AlpmPackage pkg;
    QDataStream stream(&ret,QIODevice::ReadOnly);
    stream >> pkg;

    return pkg;
}

AlpmPackage Alpm::findLocalPackage(const QString & name,const QString & version) const {
    if (!isValid()) return AlpmPackage();

    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->findLocalPackage(name,version),ret)) return AlpmPackage();

    AlpmPackage pkg;
    QDataStream stream(&ret,QIODevice::ReadOnly);
    stream >> pkg;

    return pkg;
}

QList<AlpmPackage> Alpm::findByPackageNameProvides(const AlpmPackage::Dependence & provide) const {
    if (!isValid()) return QList<AlpmPackage>();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << provide;

    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->findByPackageNameProvides(arr),ret)) return QList<AlpmPackage>();

    QList<AlpmPackage> list;
    QDataStream stream2(&ret,QIODevice::ReadOnly);
    stream2 >> list;
    return list;
}

QList<AlpmPackage> Alpm::findLocalByPackageNameProvides(const AlpmPackage::Dependence & provide) const {
    if (!isValid()) return QList<AlpmPackage>();

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << provide;

    QByteArray ret;
    if (!replyToValue<QByteArray>(m_interface->findLocalByPackageNameProvides(arr),ret)) return QList<AlpmPackage>();

    QList<AlpmPackage> list;
    QDataStream stream2(&ret,QIODevice::ReadOnly);
    stream2 >> list;
    return list;
}

void Alpm::resetPackageChangeStatuses() {
    if (!isValid()) return;

    m_interface->resetPackageChangeStatuses();
}

AlpmPackage Alpm::createLocalPackage(const QString & pkgpath) const {
    if (!isValid()) return AlpmPackage();

    QByteArray arr;
    if (!replyToValue<QByteArray>(m_interface->createLocalPackage(pkgpath),arr)) return AlpmPackage();

    AlpmPackage pkg;
    QDataStream stream(&arr,QIODevice::ReadOnly);
    stream >> pkg;

    return pkg;
}

void Alpm::deleteLocalPackage(const AlpmPackage & pkg) {
    if (!isValid()) return;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << pkg;

    replyToVoid(m_interface->deleteLocalPackage(arr));
}

void Alpm::dbRefresherIsAboutToStart() {
    replyToVoid(m_interface->dbRefresherIsAboutToStart());
}

void Alpm::updaterAboutToStart() {
    replyToVoid(m_interface->updaterAboutToStart());
}

bool Alpm::addNewRepo(const Alpm::Repo & repo) {
    if (!isValid()) return false;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << repo;

    bool ret = false;
    if (!replyToValue<bool>(m_interface->addNewRepo(arr),ret)) return ret;
    return ret;
}

bool Alpm::addMirrorRepo(const Alpm::Repo & repo) {
    if (!isValid()) return false;

    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << repo;

    bool ret = false;
    if (!replyToValue<bool>(m_interface->addMirrorRepo(arr),ret)) return ret;
    return ret;
}

bool Alpm::deleteRepo(const QString & name) {
    if (!isValid()) return false;

    bool ret = false;
    if (!replyToValue<bool>(m_interface->deleteRepo(name),ret)) return ret;
    return ret;
}
