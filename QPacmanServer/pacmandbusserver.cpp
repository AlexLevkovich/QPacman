/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmandbusserver.h"
#include <QDir>
#include "pacmanrepositoryreader.h"
#include "pacmanfileslistreader.h"
#include "pacmandbrefresher.h"
#include "pacmanfilepackageinforeader.h"
#include "pacmaninstalllocalpackagesreader.h"
#include "pacmaninstallpackagesreader.h"
#include "pacmanpackagereasonchanger.h"
#include "pacmanremovepackagesreader.h"
#include "pacmansimpleupdatesreader.h"

#define IS_TRUE(a) (a > 0)
#define IS_FALSE(a) (a == 0)
#define IS_BOOL(a) (a >= 0)
#define ARE_YOU_HERE_INTERVAL 1000

Q_DECLARE_METATYPE(QList<int>)

extern QString pacman_cache_dir;
extern QString invalid_password_str;
extern const QByteArray encryptedSalt();
extern const QByteArray encryptedUserPassword();

static bool removeDir(const QString & dirName) {
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
    }
    return result;
}

PacmanDBusServer::PacmanDBusServer(QObject *parent) :  QDBusAbstractAdaptor(parent) {
    _init();

    if(!QDBusConnection::systemBus().registerService(service())) {
        qCritical() << QDBusConnection::systemBus().lastError();
        qApp->exit(1);
        return;
    }

    if(!QDBusConnection::systemBus().registerObject(path(), this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties)) {
        qCritical() << QDBusConnection::systemBus().lastError();
        qApp->exit(2);
        return;
    }
}

PacmanDBusServer::~PacmanDBusServer() {
    QDBusConnection::systemBus().unregisterObject(path(),QDBusConnection::UnregisterTree);
}

void PacmanDBusServer::_init() {
    m_password.clear();
    m_url.clear();
    m_file_path.clear();
    m_packages_list.clear();
    m_packages.clear();
    m_doDeps = -1;
    installreader = NULL;
    removereader = NULL;
    localinstallreader = NULL;
}

QString PacmanDBusServer::commandRequest(const QByteArray & command) {
    if (m_commands.key(command,NULL) != NULL) {
        return tr("You are trying to run %1 command twice!!!").arg(QString::fromLatin1(command));
    }

    if (command == "SHOW TRAY") {
        emit_show_tray_window();
        emit_command_finished(command,"");
    }
    else if (command == "CLIENT STARTED") {
        emit_client_started();
        emit_command_finished(command,"");
    }
    else if (command == "CLIENT EXITED") {
        emit_client_exited();
        emit_command_finished(command,"");
    }
    else if (command == "FILE INFO") {
        if (m_file_path.isEmpty()) {
            return tr("Invalid input file path: empty!!!");
        }

        PacmanFilePackageInfoReader * inforeader = new PacmanFilePackageInfoReader(m_file_path);
        connect(inforeader,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(package_info_finished(PacmanProcessReader *)));
        m_commands[inforeader] = command;
    }
    else if (command == "LIST OF PACKAGES") {
        PacmanRepositoryReader * pacrepreader = new PacmanRepositoryReader(this);
        connect(pacrepreader,SIGNAL(read_package(const PacmanEntry &)),this,SLOT(emit_package_ready(const PacmanEntry &)));
        connect(pacrepreader,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(read_finished(PacmanProcessReader *)));
        m_commands[pacrepreader] = command;
    }
    else if (command == "FILES OF PACKAGES") {
        PacmanFilesListReader * filesreader = new PacmanFilesListReader(this);
        connect(filesreader,SIGNAL(files_ready(const QString &,const QStringList &)),this,SLOT(emit_files_ready(const QString &,const QStringList &)));
        connect(filesreader,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(read_finished(PacmanProcessReader *)));
        m_commands[filesreader] = command;
    }
    else if (command == "CLEAN CACHE") {
        QString err_str = invalidPasswordError(m_password);
        if (err_str.isEmpty()) {
            if (!removeDir(pacman_cache_dir)) return tr("Cannot remove the contents of directory!!!");
            else emit_command_finished(command,"");
        }
        else return err_str;
    }
    else if (command == "DB REFRESH") {
        PacmanDBRefresher * dbrefresher = new PacmanDBRefresher(this);
        connect(dbrefresher,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(refresher_finished(PacmanProcessReader *)));
        m_commands[dbrefresher] = command;
    }
    else if (command == "READ UPDATES") {
        PacmanSimpleUpdatesReader * updatesreader = new PacmanSimpleUpdatesReader(this);
        connect(updatesreader,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(simple_updates_finished(PacmanProcessReader *)));
        m_commands[updatesreader] = command;
    }
    else if (command == "SALT") {
        emit_salt_ready(encryptedSalt());
        emit_command_finished(command,"");
    }
    else if (command == "READ INSTALL PACKAGES") {
        if ((installreader != NULL) || (localinstallreader != NULL)) {
            return tr("One pacman process already is started!!!");
        }
        else if (m_packages.isEmpty()) {
            return tr("Input package list is empty!!!");
        }
        QString err_str = invalidPasswordError(m_password);
        if (err_str.isEmpty()) {
            installreader = new PacmanInstallPackagesReader(m_packages);
            connect(installreader,SIGNAL(post_messages(const QString &,const QStringList &)),this,SLOT(emit_post_messages(const QString &,const QStringList &)));
            connect(installreader,SIGNAL(ready_to_process(int)),this,SLOT(emit_ready_to_process_install(int)));
            connect(installreader,SIGNAL(start_installing(const QString &)),this,SLOT(emit_start_installing(const QString &)));
            connect(installreader,SIGNAL(start_removing(const QString &)),this,SLOT(emit_start_removing(const QString &)));
            connect(installreader,SIGNAL(some_providers_available(const QStringList &)),this,SLOT(emit_some_providers_available(const QStringList &)));
            connect(installreader,SIGNAL(question_available(const QString &)),this,SLOT(emit_question_available(const QString &)));
            connect(installreader,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(install_packages_finished(PacmanProcessReader *)));
            connect(installreader,SIGNAL(contents_length_found(int)),this,SLOT(emit_contents_length_found(int)));
            connect(installreader,SIGNAL(download_progress(int)),this,SLOT(emit_download_progress(int)));
            connect(installreader,SIGNAL(start_download(const QString &)),this,SLOT(emit_start_download(const QString &)));
            connect(installreader,SIGNAL(all_downloads_completed()),this,SLOT(emit_all_downloads_completed()));
        }
        else return err_str;
    }
    else if (command == "READ LOCAL INSTALL PACKAGES") {
        if ((installreader != NULL) || (localinstallreader != NULL)) {
            return tr("One pacman process already is started!!!");
        }
        else if (m_packages_list.isEmpty()) {
            return tr("Input package list is empty!!!");
        }
        QString err_str = invalidPasswordError(m_password);
        if (err_str.isEmpty()) {
            localinstallreader = new PacmanInstallLocalPackagesReader(m_packages_list);
            connect(localinstallreader,SIGNAL(post_messages(const QString &,const QStringList &)),this,SLOT(emit_post_messages(const QString &,const QStringList &)));
            connect(localinstallreader,SIGNAL(ready_to_process(int)),this,SLOT(emit_ready_to_process_install(int)));
            connect(localinstallreader,SIGNAL(start_installing(const QString &)),this,SLOT(emit_start_installing(const QString &)));
            connect(localinstallreader,SIGNAL(start_removing(const QString &)),this,SLOT(emit_start_removing(const QString &)));
            connect(localinstallreader,SIGNAL(some_providers_available(const QStringList &)),this,SLOT(emit_some_providers_available(const QStringList &)));
            connect(localinstallreader,SIGNAL(question_available(const QString &)),this,SLOT(emit_question_available(const QString &)));
            connect(localinstallreader,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(install_packages_finished(PacmanProcessReader *)));
            connect(localinstallreader,SIGNAL(contents_length_found(int)),this,SLOT(emit_contents_length_found(int)));
            connect(localinstallreader,SIGNAL(download_progress(int)),this,SLOT(emit_download_progress(int)));
            connect(localinstallreader,SIGNAL(start_download(const QString &)),this,SLOT(emit_start_download(const QString &)));
            connect(localinstallreader,SIGNAL(all_downloads_completed()),this,SLOT(emit_all_downloads_completed()));
        }
        else return err_str;
    }
    else if (command == "CHANGE REASON") {
        if (m_packages.isEmpty()) {
            return tr("Input package list is empty!!!");
        }
        if (!IS_BOOL(m_doDeps)) {
            return tr("Dependency flag should be boolean!!!");
        }

        QString err_str = invalidPasswordError(m_password);
        if (err_str.isEmpty()) {
            PacmanPackageReasonChanger * changer = new PacmanPackageReasonChanger(m_packages,m_doDeps);
            connect(changer,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(read_finished(PacmanProcessReader *)));
            m_commands[changer] = command;
        }
        else return err_str;
    }
    else if (command == "READ REMOVE PACKAGES") {
        if (m_packages.isEmpty()) {
            return tr("Input package list is empty!!!");
        }

        QString err_str = invalidPasswordError(m_password);
        if (err_str.isEmpty()) {
            removereader = new PacmanRemovePackagesReader(m_packages);
            connect(removereader,SIGNAL(ready_to_process(int)),this,SLOT(emit_ready_to_process_remove(int)));
            connect(removereader,SIGNAL(start_removing(const QString &)),this,SLOT(emit_start_removing(const QString &)));
            connect(removereader,SIGNAL(post_messages(const QString &,const QStringList &)),this,SLOT(emit_post_messages(const QString &,const QStringList &)));
            connect(removereader,SIGNAL(finished(PacmanProcessReader *)),this,SLOT(read_remove_packages_finished(PacmanProcessReader *)));
        }
        else return err_str;
    }
    else if (command == "PASSWORD") {
        if (m_password != encryptedUserPassword()) {
            return invalid_password_str;
        }
        else emit_command_finished(command,"");
    }
    else return tr("Invalid command!!!");

    return "";
}

void PacmanDBusServer::_completed(QObject * obj,int code,const QString & error) {
    QString m_error = (code != 0)?error:"";
    if (m_commands.contains(obj)) {
        emit_command_finished(m_commands[obj],m_error);
        m_commands.remove(obj);
        delete obj;
    }
    else if (removereader == obj) {
        emit_command_finished("READ REMOVE PACKAGES",m_error);
        delete removereader;
        removereader = NULL;
    }
    else if (installreader == obj) {
        emit_command_finished("READ INSTALL PACKAGES",m_error);
        delete installreader;
        installreader = NULL;
    }
    else if (localinstallreader == obj) {
        emit_command_finished("READ LOCAL INSTALL PACKAGES",m_error);
        delete localinstallreader;
        localinstallreader = NULL;
    }
}

void PacmanDBusServer::terminateRequest(const QByteArray & command) {
    if (command.isEmpty()) {
        terminateRequest("READ REMOVE PACKAGES");
        terminateRequest("READ INSTALL PACKAGES");
        terminateRequest("READ LOCAL INSTALL PACKAGES");
        QMapIterator<QObject *,QByteArray> i(m_commands);
        while (i.hasNext()) {
            i.next();
            terminateRequest(i.value());
        }
    }
    else if (command == "READ REMOVE PACKAGES") {
        if (removereader != NULL) {
            removereader->terminate();
        }
    }
    else if (command == "READ INSTALL PACKAGES") {
        if (installreader != NULL) {
            installreader->terminate();
        }
    }
    else if (command == "READ LOCAL INSTALL PACKAGES") {
        if (localinstallreader != NULL) {
            localinstallreader->terminate();
        }
    }
    else {
        QObject * obj = m_commands.key(command,NULL);
        if (obj != NULL) {
            if (obj->inherits("PacmanProcessReader")) ((PacmanProcessReader *)obj)->terminate();
            else QMetaObject::invokeMethod(this,"_completed",Qt::QueuedConnection,Q_ARG(QObject *,obj),Q_ARG(int,0),Q_ARG(const QString &,""));
        }
    }
}

QString PacmanDBusServer::invalidPasswordError(const QByteArray & password) const {
    if (password != encryptedUserPassword()) {
        return invalid_password_str;
    }

    return "";
}

void PacmanDBusServer::setPassword(const QByteArray & password) {
    m_password = password;
}

void PacmanDBusServer::setUrl(const QString & url) {
    m_url = QUrl(url);
}

void PacmanDBusServer::setFilePath(const QString & file_path) {
    m_file_path = file_path;
}

void PacmanDBusServer::setPackageList(const QStringList & packages) {
    m_packages_list = packages;
}

void PacmanDBusServer::setPackages(const QString & packages) {
    m_packages = packages;
}

void PacmanDBusServer::setSelectedProvider(const QString & provider) {
    if (installreader != NULL) installreader->sendChosenProvider(provider);
    else if (localinstallreader != NULL) localinstallreader->sendChosenProvider(provider);
}

void PacmanDBusServer::beginInstall() {
    if (installreader != NULL) installreader->beginInstall();
    else if (localinstallreader != NULL) localinstallreader->beginInstall();
}

void PacmanDBusServer::sendAnswer(int answer) {
    if (installreader != NULL) installreader->sendAnswer(answer);
    else if (localinstallreader != NULL) localinstallreader->sendAnswer(answer);
}

void PacmanDBusServer::cancelInstall() {
    if (installreader != NULL) installreader->cancelInstall();
    else if (localinstallreader != NULL) localinstallreader->cancelInstall();
}

void PacmanDBusServer::beginRemove() {
    if (removereader == NULL) return;
    removereader->beginRemove();
}

void PacmanDBusServer::cancelRemove() {
    if (removereader == NULL) return;
    removereader->cancelRemove();
}

void PacmanDBusServer::setDependance(bool doDeps) {
    m_doDeps = doDeps?1:0;
}

void PacmanDBusServer::read_finished(PacmanProcessReader * ptr) {
    int code = ptr->exitCode();
    if (ptr->wasTerminated()) code = 0;

    bool isFileListReader = false;
    if (ptr->inherits("PacmanFilesListReader")) isFileListReader = true;

    QMetaObject::invokeMethod(this,"_completed",Qt::QueuedConnection,Q_ARG(QObject *,ptr),Q_ARG(int,code),Q_ARG(const QString &,ptr->errorStream()));

    if (!isFileListReader) return;

    QMetaObject::invokeMethod(this,"commandRequest",Qt::QueuedConnection,Q_ARG(QByteArray,"READ UPDATES"));
}

void PacmanDBusServer::refresher_finished(PacmanProcessReader * ptr) {
    int code = ptr->exitCode();
    if (ptr->wasTerminated()) code = 0;
    QMetaObject::invokeMethod(this,"_completed",Qt::QueuedConnection,Q_ARG(QObject *,ptr),Q_ARG(int,code),Q_ARG(const QString &,ptr->errorStream()));
}

void PacmanDBusServer::simple_updates_finished(PacmanProcessReader * ptr) {
    PacmanSimpleUpdatesReader * suptr = (PacmanSimpleUpdatesReader *) ptr;
    int code = suptr->exitCode();
    if (!ptr->wasTerminated()) {
        emit_packages_to_update((code != 0)?QStringList():suptr->packages());
    }
    else code = 0;
    QMetaObject::invokeMethod(this,"_completed",Qt::QueuedConnection,Q_ARG(QObject *,ptr),Q_ARG(int,code),Q_ARG(const QString &,ptr->errorStream()));
}

void PacmanDBusServer::package_info_finished(PacmanProcessReader * ptr) {
    int code = 0;
    if (!ptr->wasTerminated()) {
        PacmanFilePackageInfoReader * piptr = (PacmanFilePackageInfoReader *) ptr;

        code = ptr->exitCode();
        if (code == 0) emit_package_ready(piptr->info());
    }
    QMetaObject::invokeMethod(this,"_completed",Qt::QueuedConnection,Q_ARG(QObject *,ptr),Q_ARG(int,code),Q_ARG(const QString &,ptr->errorStream()));
}

void PacmanDBusServer::install_packages_finished(PacmanProcessReader * ptr) {
    int code = ptr->exitCode();
    if (ptr->wasTerminated()) code = 0;
    QMetaObject::invokeMethod(this,"_completed",Qt::QueuedConnection,Q_ARG(QObject *,ptr),Q_ARG(int,code),Q_ARG(const QString &,ptr->errorStream()));
}

void PacmanDBusServer::read_remove_packages_finished(PacmanProcessReader * ptr) {
    int code = ptr->exitCode();
    if (ptr->wasTerminated()) code = 0;
    QMetaObject::invokeMethod(this,"_completed",Qt::QueuedConnection,Q_ARG(QObject *,ptr),Q_ARG(int,code),Q_ARG(const QString &,ptr->errorStream()));
}

void PacmanDBusServer::emit_command_finished(const QByteArray & command,const QString &errorMsg) {
    QList<QVariant> argumentList;
    argumentList << command << QVariant::fromValue(errorMsg);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"command_finished");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_package_ready(const PacmanEntry & entry) {
    QByteArray arr;
    QDataStream stream(&arr,QIODevice::WriteOnly);
    stream << entry;
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(arr);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"package_ready");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_files_ready(const QString & package,const QStringList & files) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(package) << QVariant::fromValue(files);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"files_ready");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_packages_to_update(const QStringList & entry) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(entry);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"packages_to_update");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_salt_ready(const QByteArray & salt) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(salt);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"salt_ready");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_contents_length_found(int len) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(len);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"contents_length_found");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_start_download(const QString & url) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(url);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"start_download");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_all_downloads_completed() {
    QList<QVariant> argumentList;
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"all_downloads_completed");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_download_progress(int percents) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(percents);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"download_progress");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_start_installing(const QString & name) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(name);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"start_installing");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_post_messages(const QString & package,const QStringList & messages) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(package) << QVariant::fromValue(messages);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"post_messages");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_ready_to_process_install(int) {
    PacmanInstallPackagesReader * installreader = this->installreader;
    if (installreader == NULL) installreader = localinstallreader;
    if (installreader == NULL) return;

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(installreader->install_packages()) <<
                    QVariant::fromValue(installreader->remove_packages()) <<
                    (installreader->inherits("PacmanInstallLocalPackagesReader")?
                     QVariant::fromValue(((PacmanInstallLocalPackagesReader *)installreader)->local_install_packages()):
                     QVariant::fromValue(QStringList())) << installreader->total_installed() << installreader->total_removed();
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"ready_to_process_install");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_ready_to_process_remove(int) {
    if (removereader == NULL) return;

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(removereader->packages()) << removereader->total_removed();
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"ready_to_process_remove");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_start_removing(const QString & name) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(name);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"start_removing");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_some_providers_available(const QStringList & providers) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(providers);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"some_providers_available");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_question_available(const QString & question) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(question);
    QDBusMessage msg = QDBusMessage::createSignal(path(),service(),"question_available");
    msg.setArguments(argumentList);
    QDBusConnection::systemBus().send(msg);
}

void PacmanDBusServer::emit_show_tray_window() {
    QDBusConnection::systemBus().send(QDBusMessage::createSignal(path(),service(),"show_tray_window"));
}

void PacmanDBusServer::emit_client_started() {
    QDBusConnection::systemBus().send(QDBusMessage::createSignal(path(),service(),"client_started"));
}

void PacmanDBusServer::emit_client_exited() {
    QDBusConnection::systemBus().send(QDBusMessage::createSignal(path(),service(),"client_exited"));
}
