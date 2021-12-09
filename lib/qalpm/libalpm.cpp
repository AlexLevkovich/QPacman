/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "libalpm.h"
#include "alpmlist.h"
#include <fnmatch.h>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDirIterator>
#include "alpmdownloader.h"
#ifdef USE_QDBUS
#include <QDBusMetaType>
#endif
#include <QDebug>
#include <alpm.h>

Alpm * Alpm::p_alpm = NULL;
int Alpm::m_percent = -1;
int Alpm::prev_event_type = -1;
AlpmConfig * Alpm::p_config = NULL;
QStringList Alpm::m_download_errs;

enum Errors {
    PKG_LIST_IS_EMPTY             = (ALPM_ERR_OK - 14),
    ALPM_INSTANCE_ALREADY_CREATED = (ALPM_ERR_OK - 13),
    ALPM_IS_NOT_OPEN              = (ALPM_ERR_OK - 12),
    PKG_IS_NOT_INITED             = (ALPM_ERR_OK - 11),
    REASON_WRONG_DB               = (ALPM_ERR_OK - 10),
    CANNOT_GET_ROOT               = (ALPM_ERR_OK - 9),
    ALPM_CONFIG_FAILED            = (ALPM_ERR_OK - 8),
    ALPM_LINK_LOCAL_DB_FAILED     = (ALPM_ERR_OK - 7),
    ALPM_HANDLE_FAILED            = (ALPM_ERR_OK - 6),
    THREAD_IS_STILL_RUNNING       = (ALPM_ERR_OK - 5),
    CANNOT_LOAD_CONFIG            = (ALPM_ERR_OK - 4),
    USER_REFUSAL                  = (ALPM_ERR_OK - 3),
    NOTHING_DOWNLOAD              = (ALPM_ERR_OK - 2),
    OK_CODE_SUPPRESS_ALPMS        = (ALPM_ERR_OK - 1)
};

class depend_t : public alpm_depend_t {
public:
    depend_t(const AlpmPackage::Dependence & dep) {
        this->name = strdup(dep.name().toLatin1().constData());
        this->version = strdup(dep.version().toLatin1().constData());
        this->desc = strdup(dep.description().toLocal8Bit().constData());
        this->mod = compareoper_to_mod(dep.operation());
        this->name_hash = hash_from_name();
    }

private:
    static alpm_depmod_t compareoper_to_mod(AlpmPackage::CompareOper mod) {
        switch (mod) {
        case AlpmPackage::UNKNOWN:
            return ALPM_DEP_MOD_ANY;
        case AlpmPackage::EQUAL:
            return ALPM_DEP_MOD_EQ;
        case AlpmPackage::MORE_OR_EQUAL:
            return ALPM_DEP_MOD_GE;
        case AlpmPackage::LESS_OR_EQUAL:
            return ALPM_DEP_MOD_LE;
        case AlpmPackage::MORE:
            return ALPM_DEP_MOD_GT;
        case AlpmPackage::LESS:
            return ALPM_DEP_MOD_LT;
        default:
            break;
        }
        return ALPM_DEP_MOD_ANY;
    }

    unsigned long hash_from_name() const {
        unsigned long hash = 0;
        int c;

        const char * str = name;

        if(!str) {
            return hash;
        }
        while((c = *str++)) {
            hash = c + hash * 65599;
        }

        return hash;
    }
};

template<class ForwardIt, class T, class Compare> static ForwardIt binary_search_ex(ForwardIt first, ForwardIt last, const T& value, Compare comp) {
    ForwardIt it = std::lower_bound(first, last, value, comp);
    if ((it == last) || comp(value,*it)) it = last;
    return it;
}

Alpm::Alpm(QObject *parent) : ThreadRun(parent) {
    m_alpm_handle = NULL;
    m_alpm_errno = ALPM_ERR_OK;
    if (p_alpm == NULL) p_alpm = this;
    m_percent = -1;
    prev_event_type = -1;
    m_question_loop = NULL;

#ifdef USE_QDBUS
    qDBusRegisterMetaType<StringStringMap>();
#endif

    connect(this,SIGNAL(method_finished(const QString&,const QVariant&,ThreadRun::RC)),this,SLOT(on_method_finished(const QString&,const QVariant&,ThreadRun::RC)));
    connect(&lock_watcher,SIGNAL(fileChanged(const QString &)),this,SLOT(lockFileChanged(const QString &)));
}

Alpm::~Alpm() {
    close();
    p_alpm = NULL;
}

void Alpm::on_method_finished(const QString & name,const QVariant & result,ThreadRun::RC rc) {
    if (name == "Alpm::query_packages") emit method_finished(name,(rc != ThreadRun::OK)?QList<AlpmPackage>():result.value<QList<AlpmPackage> >(),rc);
    else if (name == "Alpm::download_packages") emit method_finished(name,(rc != ThreadRun::OK)?QStringList():result.value<QStringList>(),rc);
}

void Alpm::lockFileChanged(const QString & path) {
    if (path != lockFilePath()) return;
    emit locking_changed(path,QFile(path).exists());
    if (!lock_watcher.files().contains(path)) lock_watcher.addPath(path);
}

bool Alpm::emit_event(const char *member,QGenericArgument val0,QGenericArgument val1,QGenericArgument val2,QGenericArgument val3,QGenericArgument val4,QGenericArgument val5,QGenericArgument val6,QGenericArgument val7,QGenericArgument val8,QGenericArgument val9) {
    return QMetaObject::invokeMethod(this,member,Qt::QueuedConnection,val0,val1,val2,val3,val4,val5,val6,val7,val8,val9);
}

void Alpm::emit_information(const QString & message,bool significant) {
    if (!message.isEmpty()) emit_event("information",Q_ARG(QString,message),Q_ARG(bool,significant));
}

void Alpm::emit_error(const QString & message) {
    if (!message.isEmpty()) emit_event("error",Q_ARG(QString,message));
}

class QFlagEventLoop : public QEventLoop {
public:
    QFlagEventLoop() : QEventLoop() {
        m_value = 0;
    }

    void setValue(int value) {
        m_value = value;
    }

    int value() const {
        return m_value;
    }
private:
    int m_value;
};

int Alpm::wait_for_answer() {
    QFlagEventLoop loop;
    m_question_loop = &loop;
    loop.exec();
    m_question_loop = NULL;
    return loop.value();
}

void Alpm::answer(uint value) {
    QMetaObject::invokeMethod(this,"stop_waiting",Qt::QueuedConnection,Q_ARG(int,(int)value));
}

void Alpm::stop_waiting(int value) {
    if (m_question_loop == NULL) return;
    m_question_loop->setValue(value);
    m_question_loop->quit();
}

void Alpm::emit_question(const QString & message,bool * answer) {
    emit_event("question",Q_ARG(QString,message));
    *answer = (wait_for_answer() != 0);
    emit_information(message+(*answer?": Y":": N"));
}

void Alpm::emit_progress(const char * signal,const QString & pkg_name,int percent,int n_targets,int current_target) {
    emit_event(signal,Q_ARG(QString,pkg_name),Q_ARG(int,percent),Q_ARG(int,n_targets),Q_ARG(int,current_target));
}

void Alpm::emit_progress(const char * signal,int percent) {
    emit_event(signal,Q_ARG(int,percent));
}

void Alpm::emit_select_provider(const QString & pkgname,const QStringList & providers,int * answer) {
    emit_event("select_provider",Q_ARG(QString,pkgname),Q_ARG(QStringList,providers));
    *answer = wait_for_answer();
    if (*answer < 0) *answer = 0;
    QString info = tr("You need to pick up what provider to use for %1 package:\n").arg(pkgname);
    for (int i=1;i<=providers.count();i++) {
        info += QString("%1) ").arg(i) + providers.at(i-1) + "\n";
    }
    info += tr("Select the item number: %1").arg((answer == NULL)?"":QString("%1").arg(*answer));
    emit_information(info);
}

void Alpm::emit_optdepends_event(const QString & pkgname,const StringStringMap & installed_deps,const StringStringMap & pending_deps) {
    emit_event("optdepends_event",Q_ARG(QString,pkgname),Q_ARG(StringStringMap,installed_deps),Q_ARG(StringStringMap,pending_deps));
}

bool Alpm::string_name_cmp(const QString & item1,const QString & item2) {
    QString name1;
    QString name2;
    QString ver;
    AlpmPackage::parseNameVersion(item1,name1,ver);
    AlpmPackage::parseNameVersion(item2,name2,ver);
    return (name1.compare(name2) < 0);
}

void Alpm::emit_install_packages_confirmation(const QStringList & _install,const QStringList & _remove,qint64 dl_size,qint64 install_size,qint64 remove_size,bool * ok) {
    QStringList install = _install;
    QStringList remove = _remove;
    std::sort(install.begin(),install.end(),string_name_cmp);
    std::sort(remove.begin(),remove.end(),string_name_cmp);
    emit_event("install_packages_confirmation",Q_ARG(QStringList,install),Q_ARG(QStringList,remove),Q_ARG(qint64,dl_size),Q_ARG(qint64,install_size),Q_ARG(qint64,remove_size));
    emit_event("full_download_size_found",Q_ARG(qint64,dl_size));
    QString info;
    if (remove.count() > 0) {
        info += tr("The following packages will be removed:\n");
        info += remove.join("\n");
        info += "\n";
    }
    info += tr("The following packages will be installed:\n");
    info += install.join("\n");
    info += "\n";
    info += tr("Do you agree? : ");
    *ok = (wait_for_answer() != 0);
    info += (*ok?"Y":"N");
    p_alpm->emit_information(info);
}

void Alpm::emit_remove_packages_confirmation(const QStringList & _remove,qint64 remove_size,bool * ok) {
    QStringList remove = _remove;
    std::sort(remove.begin(),remove.end(),string_name_cmp);
    emit_event("remove_packages_confirmation",Q_ARG(QStringList,remove),Q_ARG(qint64,remove_size));
    *ok = (wait_for_answer() != 0);
    QString info = tr("The following packages will be removed:\n");
    info += remove.join("\n");
    info += "\n";
    info += tr("Do you agree? : ") + (*ok?"Y":"N");
    p_alpm->emit_information(info);
}

bool Alpm::open(const QString & confpath,const QString & dbpath) {
    m_alpm_errno = ALPM_ERR_OK;
    if (isValid() || p_alpm != this) {
        m_alpm_errno = ALPM_INSTANCE_ALREADY_CREATED;
        return false;
    }

    if (!m_config.setConfPath(confpath)) {
        m_alpm_errno = CANNOT_LOAD_CONFIG;
        return false;
    }
    p_config = &m_config;

    m_alpm_handle = m_config.translate(dbpath);
    if (!isValid()) {
        m_alpm_errno = ALPM_HANDLE_FAILED;
        return false;
    }

    alpm_option_set_progresscb(m_alpm_handle,(void (*)(void *,alpm_progress_t, const char *, int, size_t, size_t))operation_progress_fn,NULL);
    alpm_option_set_questioncb(m_alpm_handle,operation_question_fn,NULL);
    alpm_option_set_eventcb(m_alpm_handle,operation_event_fn,NULL);
    alpm_option_set_fetchcb(m_alpm_handle,operation_fetch_fn,NULL);

    lock_watcher.addPath(lockFilePath());
    recreatedbs();

    return true;
}

alpm_list_t * Alpm::convert_list(const QStringList & list) {
    alpm_list_t * ret = NULL;

    for (int i=0;i<list.count();i++) {
        ret = alpm_list_add(ret,strdup(list[i].toLocal8Bit().constData()));
    }

    return ret;
}

bool Alpm::add_sync_db(const AlpmConfig::Repo & repo) {
    alpm_db_t *db_handle = alpm_register_syncdb(m_alpm_handle,repo.name().toLocal8Bit().constData(),repo.siglevel());
    if (db_handle) {
        alpm_db_set_servers(db_handle,convert_list(repo.servers()));
        alpm_db_set_usage(db_handle,repo.usage());
    }
    else {
        m_alpm_errno = alpm_errno(m_alpm_handle);
        return false;
    }

    AlpmDB db(db_handle);
    m_syncDBs.append(db);
    m_groups += db.groups();
    AlpmPackage res;
    for (AlpmPackage & pkg: db.packages()) {
        if (pkg.isIgnorable()) continue;
        for (AlpmPackage::Dependence & dep: pkg.replaces()) {
            res = m_localDB.findByPackageName(dep.name());
            if (!res.isValid()) continue;
            if (dep.isAppropriate(res)) m_replaces[res] << pkg;
        }
    }
    m_groups.removeDuplicates();
    return true;
}

bool Alpm::remove_sync_db(const QString & db_name) {
    bool ret = false;
    QList<AlpmPackage> list;
    for (int j=(m_syncDBs.count()-1);j>=0;j--) {
        AlpmDB & db = m_syncDBs[j];
        if (db.name() == db_name) {
            alpm_db_unregister(db.m_db_handle);
            m_syncDBs.removeAt(j);
            QMapIterator<AlpmPackage,QList<AlpmPackage> > i(m_replaces);
            while (i.hasNext()) {
                i.next();
                list = i.value();
                for (int k=(list.count()-1);k>=0;k--) {
                    if (list[k].repo() == db_name) {
                        list.removeAt(k);
                        if (list.isEmpty()) m_replaces.remove(i.key());
                        else m_replaces[i.key()] = list;
                        break;
                    }
                }
            }
            ret = true;
            break;
        }
    }

    if (ret) {
        m_groups.clear();
        for (AlpmDB & db: m_syncDBs) {
            m_groups += db.groups();
        }
        m_groups.removeDuplicates();
    }

    return ret;
}

void Alpm::recreatedbs() {
    m_localDB = AlpmDB(alpm_get_localdb(m_alpm_handle));
    m_syncDBs.clear();
    m_groups.clear();
    m_replaces.clear();
    m_groups += m_localDB.groups();

    AlpmDB db;
    AlpmPackage res;
    AlpmList<alpm_db_t> dbs(alpm_get_syncdbs(m_alpm_handle),AlpmList<alpm_db_t>::ignorefree);
    do {
        if (dbs.isEmpty()) break;

        db = AlpmDB(dbs.valuePtr());
        m_syncDBs.append(db);
        m_groups += db.groups();

        for (AlpmPackage & pkg: db.packages()) {
            if (pkg.isIgnorable()) continue;
            for (AlpmPackage::Dependence & dep: pkg.replaces()) {
                res = m_localDB.findByPackageName(dep.name());
                if (!res.isValid()) continue;
                if (dep.isAppropriate(res)) m_replaces[res] << pkg;
            }
        }
    } while (dbs.goNext());
    dbs.detach();

    m_groups.removeDuplicates();
}

bool Alpm::isOpen() {
    return (p_alpm != NULL && p_alpm->m_alpm_handle != NULL);
}

bool Alpm::reopen() {
    m_alpm_errno = ALPM_ERR_OK;
    if (!isValid(true)) {
        m_alpm_errno = ALPM_IS_NOT_OPEN;
        return false;
    }
    QString dbpath = m_config.dbPath();
    QString confpath = m_config.confPath();
    if (!close()) return false;
    return open(confpath,dbpath);
}

bool Alpm::close() {
    m_alpm_errno = ALPM_ERR_OK;

    if (!isValid()) {
        m_alpm_errno = ALPM_IS_NOT_OPEN;
        return false;
    }
    answer(0);

    bool ok = false;
    for (int i=0;i<30;i++) {
        if (!ThreadRun::isMethodExecuting()) {
            ok = true;
            break;
        }
        answer(0);
        qApp->processEvents(QEventLoop::AllEvents,100);
    }
    if (!ok && ThreadRun::isMethodExecuting()) {
        m_alpm_errno = THREAD_IS_STILL_RUNNING;
        return false;
    }

    lock_watcher.removePaths(lock_watcher.files());
    alpm_release(m_alpm_handle);
    m_alpm_handle = NULL;
    m_groups.clear();
    AlpmPackage::m_change_statuses.clear();
    return true;
}

Alpm * Alpm::instance() {
    return p_alpm;
}

AlpmConfig * Alpm::config() {
    return p_config;
}

int Alpm::operation_fetch_fn(void *,const QString & url,const QString & localpath,bool) {
    QUrl _url(url);
    bool is_notdb = (QFileInfo(_url.fileName()).suffix().toLower() != "db");
    if (is_notdb) {
        p_alpm->emit_information(QObject::tr("Starting the download of %1").arg(_url.fileName()));
        p_alpm->emit_event("download_start",Q_ARG(QString,_url.fileName()));
    }
    AlpmDownloader * downloader = new AlpmDownloader(_url,localpath,AlpmConfig::downloaderThreadCount(),p_config->doDisableDownloadTimeout()?0:AlpmConfig::downloaderTimeout(),AlpmConfig::downloaderProxy());
    QObject::connect(downloader,SIGNAL(progress(const QString &,qint64,qint64,int,qint64)),p_alpm,SLOT(operation_download_fn(const QString &,qint64,qint64,int,qint64)));
    QObject::connect(downloader,&AlpmDownloader::error,p_alpm,[&](const QString & err) {m_download_errs.append(err);p_alpm->emit_information(err);p_alpm->emit_error(err);});
    int ret = downloader->exec();
    delete downloader;
    if (is_notdb) {
        if (ret == 0) {
            p_alpm->emit_information(QObject::tr("Done the download of %1").arg(_url.fileName()));
            p_alpm->emit_event("download_done",Q_ARG(QString,_url.fileName()));
        }
        else {
            p_alpm->m_alpm_errno = ALPM_ERR_EXTERNAL_DOWNLOAD;
            p_alpm->emit_information(QObject::tr("Failed the download of %1").arg(_url.fileName()));
            p_alpm->emit_event("download_failed",Q_ARG(QString,_url.fileName()));
        }
    }
    return (ret == 2)?-1:ret;
}

void Alpm::operation_download_fn(const QString & filename,qint64 bytes_downloaded,qint64 length,int,qint64) {
    if(bytes_downloaded == 0 && length <= 0) return;
    if (length < 0) length = 0;
    emit information(tr("(%1/%2) downloaded").arg(bytes_downloaded).arg(length));
    emit download_progress(filename,bytes_downloaded,length);
}

int Alpm::operation_fetch_fn(void *,const char *url, const char *localpath,int force) {
    return (int)Alpm::operation_fetch_fn(NULL,QString::fromLocal8Bit(url),QString::fromLocal8Bit(localpath),(int)force);
}

void Alpm::operation_progress_fn(void *,int progress, const char * pkg_name, int percent, size_t n_targets, size_t current_target) {
    if (m_percent != -1 && m_percent == percent) return;

    m_percent = percent;
    QString pkgname = QString::fromLocal8Bit((pkg_name == NULL)?"":pkg_name);
    switch ((alpm_progress_t)progress) {
    case ALPM_PROGRESS_ADD_START:
        p_alpm->emit_information(tr("(%1/%2) Installing %3 (%4%)").arg(current_target).arg(n_targets).arg(pkgname).arg(percent));
        p_alpm->emit_progress("install_progress",pkgname,percent,(int)n_targets,(int)current_target);
        break;
    case ALPM_PROGRESS_REINSTALL_START:
        p_alpm->emit_information(tr("(%1/%2) Reinstalling %3 (%4%)").arg(current_target).arg(n_targets).arg(pkgname).arg(percent));
        p_alpm->emit_progress("install_progress",pkgname,percent,(int)n_targets,(int)current_target);
        break;
    case ALPM_PROGRESS_UPGRADE_START:
        p_alpm->emit_information(tr("(%1/%2) Upgrading %3 (%4%)").arg(current_target).arg(n_targets).arg(pkgname).arg(percent));
        p_alpm->emit_progress("install_progress",pkgname,percent,(int)n_targets,(int)current_target);
        break;
    case ALPM_PROGRESS_DOWNGRADE_START:
        p_alpm->emit_information(tr("(%1/%2) Downgrading %3 (%4%)").arg(current_target).arg(n_targets).arg(pkgname).arg(percent));
        p_alpm->emit_progress("install_progress",pkgname,percent,(int)n_targets,(int)current_target);
        break;
    case ALPM_PROGRESS_REMOVE_START:
        p_alpm->emit_information(tr("(%1/%2) Removing %3 (%4%)").arg(current_target).arg(n_targets).arg(pkgname).arg(percent));
        p_alpm->emit_progress("remove_progress",pkgname,percent,(int)n_targets,(int)current_target);
        break;
    case ALPM_PROGRESS_CONFLICTS_START:
        p_alpm->emit_information(QString("%1% %2").arg(percent).arg(tr("checked")));
        p_alpm->emit_progress("conflicts_progress",percent);
        break;
    case ALPM_PROGRESS_DISKSPACE_START:
        p_alpm->emit_information(QString("%1% %2").arg(percent).arg(tr("checked")));
        p_alpm->emit_progress("diskspace_progress",percent);
        break;
    case ALPM_PROGRESS_INTEGRITY_START:
        p_alpm->emit_information(QString("%1% %2").arg(percent).arg(tr("checked")));
        p_alpm->emit_progress("integrity_progress",percent);
        break;
    case ALPM_PROGRESS_LOAD_START:
        p_alpm->emit_information(QString("%1% %2").arg(percent).arg(tr("loaded")));
        p_alpm->emit_progress("load_progress",percent);
        break;
    case ALPM_PROGRESS_KEYRING_START:
        p_alpm->emit_information(QString("%1% %2").arg(percent).arg(tr("checked")));
        p_alpm->emit_progress("keyring_progress",percent);
        break;
    }
}

const QString Alpm::alpm_item_string_fn(alpm_pkg_t * value) {
    return QString::fromLocal8Bit((const char *)alpm_pkg_get_name(value));
}

void Alpm::operation_question_fn(void *,alpm_question_t * question) {
    bool answer;
    switch(question->type) {
        case ALPM_QUESTION_INSTALL_IGNOREPKG:
            p_alpm->emit_question(QObject::tr("%1 is in IgnorePkg/IgnoreGroup. Install anyway?").arg(QString::fromLocal8Bit(alpm_pkg_get_name(question->install_ignorepkg.pkg))),
                                                                                                 &answer);
            question->install_ignorepkg.install = answer?1:0;
            break;
        case ALPM_QUESTION_REPLACE_PKG:
            p_alpm->emit_question(QObject::tr("Replace %1 with %2/%3?").arg(QString::fromLocal8Bit(alpm_pkg_get_name(question->replace.oldpkg))).
                                                                                           arg(QString::fromLocal8Bit(alpm_db_get_name(question->replace.newdb))).
                                                                                           arg(QString::fromLocal8Bit(alpm_pkg_get_name(question->replace.newpkg))),
                                                                                           &answer);
            question->replace.replace = answer?1:0;
            break;
        case ALPM_QUESTION_CONFLICT_PKG:
            if(!strcmp((const char *)question->conflict.conflict->package1,(const char *)question->conflict.conflict->reason->name) || !strcmp((const char *)question->conflict.conflict->package2,(const char *)question->conflict.conflict->reason->name)) {
                p_alpm->emit_question(QObject::tr("%1 and %2 are in conflict. Remove %3?").arg(QString::fromLocal8Bit((const char *)question->conflict.conflict->package1)).
                                                                                           arg(QString::fromLocal8Bit((const char *)question->conflict.conflict->package2)).
                                                                                           arg(QString::fromLocal8Bit((const char *)question->conflict.conflict->package2)),
                                                                                           &answer);
            } else {
                p_alpm->emit_question(QObject::tr("%1 and %2 are in conflict (%3-%4). Remove %5?").arg(QString::fromLocal8Bit((const char *)question->conflict.conflict->package1)).
                                                                                                   arg(QString::fromLocal8Bit((const char *)question->conflict.conflict->package2)).
                                                                                                   arg(QString::fromLocal8Bit((const char *)question->conflict.conflict->reason->name)).
                                                                                                   arg(QString::fromLocal8Bit((const char *)question->conflict.conflict->reason->version)).
                                                                                                   arg(QString::fromLocal8Bit((const char *)question->conflict.conflict->package2)),
                                                                                                   &answer);
            }
            question->conflict.remove = answer?1:0;
            break;
        case ALPM_QUESTION_REMOVE_PKGS:
            {
                AlpmList<alpm_pkg_t> namelist(question->remove_pkgs.packages);
                QString message(QObject::tr("The following packages cannot be upgraded due to unresolvable dependencies:\n"));
                message += namelist.toString(alpm_item_string_fn,QString("     "));
                message += QObject::tr("Do you want to skip upgrading these packages?");
                p_alpm->emit_question(message,&answer);
                question->remove_pkgs.skip = answer?1:0;
                namelist.detach();
            }
            break;
        case ALPM_QUESTION_SELECT_PROVIDER:
            {
                AlpmList<alpm_pkg_t> namelist(question->select_provider.providers);
                QStringList list;
                do {
                    if (namelist.isEmpty()) break;
                    alpm_pkg_t * pkg = namelist.valuePtr();
                    alpm_db_t * db = alpm_pkg_get_db(pkg);
                    QString db_name;
                    if (db != NULL) db_name = QString::fromLocal8Bit((const char *)alpm_db_get_name(db));
                    QString pkg_name = QString::fromLocal8Bit((const char *)alpm_pkg_get_name(pkg));
                    list.append((db_name.isEmpty()?pkg_name:db_name+"/"+pkg_name)+"-"+QString::fromLatin1((const char *)alpm_pkg_get_version(pkg)));
                } while (namelist.goNext());
                namelist.detach();

                p_alpm->emit_select_provider(QString::fromLocal8Bit(question->select_provider.depend->name),list,&question->select_provider.use_index);

            }
            return;
        case ALPM_QUESTION_CORRUPTED_PKG:
            p_alpm->emit_question(QObject::tr("File %1 is corrupted.\nDo you want to delete it?").arg(QString::fromLocal8Bit(question->corrupted.filepath)),&answer);
            question->corrupted.remove = answer?1:0;
            break;
        case ALPM_QUESTION_IMPORT_KEY:
            {
                alpm_pgpkey_t *key = question->import_key.key;
                char created[12];
                const char *revoked = "";
                time_t time = (time_t)key->created;
                strftime(created, 12, "%Y-%m-%d", localtime(&time));

                if(key->revoked) {
                    revoked = " (revoked)";
                }

                p_alpm->emit_question(QObject::tr("Import PGP key %1%2/%3, \"%4\", created: %5%6?").arg(key->length).
                                                                                                    arg(QChar::fromLatin1((char)key->pubkey_algo)).
                                                                                                    arg(QString::fromLocal8Bit((const char *)key->fingerprint)).
                                                                                                    arg(QString::fromLocal8Bit((const char *)key->uid)).
                                                                                                    arg(QString::fromLocal8Bit((const char *)created)).
                                                                                                    arg(QString::fromLocal8Bit((const char *)revoked)),
                                                                                                    &answer);
                question->import_key.import = answer?1:0;
            }
            break;
    }
}

const QString Alpm::dep_item_add(alpm_depend_t * optdep,StringStringMap & installed_deps,StringStringMap & pending_deps) {
    char * tmpstr;
    QString optstring = QString::fromLocal8Bit((const char *)(tmpstr = (char *)alpm_dep_compute_string(optdep)));
    free(tmpstr);
    QString depname = QString::fromLocal8Bit(optdep->name)+"="+QString::fromLocal8Bit(optdep->version);
    QString desc = QString::fromLocal8Bit(optdep->desc);
    if(alpm_find_satisfier(alpm_db_get_pkgcache(p_alpm->localDB().m_db_handle),optstring.toLocal8Bit().constData())) installed_deps.insert(depname,desc);
    else pending_deps.insert(depname,desc);
    return optstring;
}

void Alpm::display_optdepends(alpm_pkg_t * pkg) {
    alpm_list_t * optdeps = alpm_pkg_get_optdepends(pkg);
    if (optdeps == NULL) return;

    QString pkgname = QString::fromLocal8Bit(alpm_pkg_get_name(pkg));
    QString info = tr("%1 has the following optional dependencies you may install:\n").arg(pkgname);
    StringStringMap installed_deps;
    StringStringMap pending_deps;
    AlpmList<alpm_depend_t> depends(optdeps);
    do {
        if (depends.count() <= 0) return;
        info += dep_item_add(depends.valuePtr(),installed_deps,pending_deps) + "\n";
    } while(depends.goNext());
    depends.detach();

    p_alpm->emit_information(info);
    p_alpm->emit_optdepends_event(pkgname,installed_deps,pending_deps);
}

int Alpm::depend_cmp(const void *d1, const void *d2) {
    const alpm_depend_t *dep1 = (const alpm_depend_t *)d1;
    const alpm_depend_t *dep2 = (const alpm_depend_t *)d2;
    int ret;

    ret = strcmp(dep1->name, dep2->name);
    if(ret == 0) {
        ret = dep1->mod - dep2->mod;
    }
    if(ret == 0) {
        if(dep1->version && dep2->version) {
            ret = strcmp(dep1->version, dep2->version);
        } else if(!dep1->version && dep2->version) {
            ret = -1;
        } else if(dep1->version && !dep2->version) {
            ret = 1;
        }
    }
    if(ret == 0) {
        if(dep1->desc && dep2->desc) {
            ret = strcmp(dep1->desc, dep2->desc);
        } else if(!dep1->desc && dep2->desc) {
            ret = -1;
        } else if(dep1->desc && !dep2->desc) {
            ret = 1;
        }
    }

    return ret;
}

void Alpm::display_new_optdepends(alpm_pkg_t *oldpkg, alpm_pkg_t *newpkg) {
    alpm_list_t * _old = alpm_pkg_get_optdepends(oldpkg);
    alpm_list_t * _new = alpm_pkg_get_optdepends(newpkg);
    if (_old == NULL || _new == NULL) return;

    QString pkgname = QString::fromLocal8Bit(alpm_pkg_get_name(newpkg));
    QString info = tr("%1 has the following optional new dependencies you may install:\n").arg(pkgname);
    StringStringMap installed_deps;
    StringStringMap pending_deps;
    AlpmList<alpm_depend_t> depends(alpm_list_diff(_new,_old,depend_cmp),AlpmList<alpm_depend_t>::ignorefree);
    do {
        if (depends.count() <= 0) return;
        info += dep_item_add(depends.valuePtr(),installed_deps,pending_deps) + "\n";
    } while(depends.goNext());

    p_alpm->emit_information(info);
    p_alpm->emit_optdepends_event(pkgname,installed_deps,pending_deps);
}

class OverwriteHandler {
private:
    OverwriteHandler(const QList<AlpmPackage> & forcedpkgs) {
        m_forcedpkgs = forcedpkgs;
    }

    ~OverwriteHandler() {
        if (Alpm::instance() == NULL || Alpm::instance()->m_alpm_handle == NULL) return;

        AlpmList<char> overwrite_files(alpm_list_copy(alpm_option_get_overwrite_files(Alpm::instance()->m_alpm_handle)),AlpmList<char>::ignorefree);
        do {
            if (overwrite_files.isEmpty()) break;
            alpm_option_remove_overwrite_file(Alpm::instance()->m_alpm_handle,(const char *)overwrite_files.valuePtr());
        } while(overwrite_files.goNext());
        m_forcedpkgs.clear();
    }

    static void addPkgFiles() {
        if (Alpm::instance() == NULL || Alpm::instance()->m_alpm_handle == NULL) return;

        QString pkg_cache_path;
        for (int i=0;i<m_forcedpkgs.count();i++) {
            pkg_cache_path.clear();
            if (!m_forcedpkgs[i].isDownloaded(&pkg_cache_path) || pkg_cache_path.isEmpty()) continue;
            addFiles(AlpmPackage(pkg_cache_path).files());
        }
    }

    static void addFiles(const QList<AlpmPackage::FileInfo> & files) {
        for (int i=0;i<files.count();i++) {
            QFileInfo info(files[i].path());
            if (!info.exists() || (!info.isFile() && !info.isSymLink()) || (info.isDir() && !info.isSymLink())) continue;
            alpm_option_add_overwrite_file(Alpm::instance()->m_alpm_handle,files.at(i).path().toLocal8Bit().constData());
        }
    }

    static QList<AlpmPackage> m_forcedpkgs;
    friend class Alpm;
};

QList<AlpmPackage> OverwriteHandler::m_forcedpkgs;

void Alpm::operation_event_fn(void *,alpm_event_t * event) {
    m_percent = -1;
    p_alpm->m_alpm_errno = ALPM_ERR_OK;

    if ((prev_event_type == ALPM_EVENT_SCRIPTLET_INFO) && (event->type != ALPM_EVENT_SCRIPTLET_INFO)) p_alpm->emit_event("scriplet_executed");

    switch(event->type) {
        case ALPM_EVENT_DB_RETRIEVE_START:
        case ALPM_EVENT_DB_RETRIEVE_DONE:
        case ALPM_EVENT_DB_RETRIEVE_FAILED:
            break;
        case ALPM_EVENT_HOOK_START:
        {
            QString message = (event->hook.when == ALPM_HOOK_PRE_TRANSACTION)?QObject::tr("Running pre-transaction hooks..."):QObject::tr("Running post-transaction hooks...");
            p_alpm->emit_event("all_hooks",Q_ARG(QString,message));
            p_alpm->emit_information(message);
            break;
        }
        case ALPM_EVENT_HOOK_RUN_START:
        {
            alpm_event_hook_run_t *e = &event->hook_run;
            QString txt = e->desc?QString::fromLocal8Bit(e->desc):QString::fromLocal8Bit(e->name);
            p_alpm->emit_event("hook",Q_ARG(QString,txt),Q_ARG(int,(int)e->position),Q_ARG(int,(int)e->total));
            p_alpm->emit_information(QString("%1/%2 %3").arg(e->position).arg(e->total).arg(txt));
            break;
        }
        case ALPM_EVENT_CHECKDEPS_START:
        {
            QString message = QObject::tr("Checking dependencies...");
            p_alpm->emit_event("checking_pkg_deps",Q_ARG(QString,message));
            p_alpm->emit_information(message);
            break;
        }
        case ALPM_EVENT_FILECONFLICTS_START:
        {
            QString message = QObject::tr("Checking for file conflicts...");
            p_alpm->emit_event("checking_file_conflicts",Q_ARG(QString,message));
            p_alpm->emit_information(message);
            OverwriteHandler::addPkgFiles();
            break;
        }
        case ALPM_EVENT_RESOLVEDEPS_START:
        {
            QString message = QObject::tr("Resolving dependencies...");
            p_alpm->emit_event("resolving_pkg_deps",Q_ARG(QString,message));
            p_alpm->emit_information(message);
            break;
        }
        case ALPM_EVENT_INTERCONFLICTS_START:
        {
            QString message = QObject::tr("Looking for conflicting packages...");
            p_alpm->emit_event("checking_internal_conflicts",Q_ARG(QString,message));
            p_alpm->emit_information(message);
            break;
        }
        case ALPM_EVENT_TRANSACTION_START:
            p_alpm->emit_information(QObject::tr("Processing package changes..."));
            break;
        case ALPM_EVENT_PACKAGE_OPERATION_START:
            break;
        case ALPM_EVENT_PACKAGE_OPERATION_DONE:
        {
            alpm_event_package_operation_t *e = &event->package_operation;
            switch(e->operation) {
            case ALPM_PACKAGE_INSTALL:
                display_optdepends(e->newpkg);
                AlpmPackage::m_change_statuses.remove(e->newpkg);
                break;
            case ALPM_PACKAGE_UPGRADE:
            case ALPM_PACKAGE_DOWNGRADE:
                display_new_optdepends(e->oldpkg, e->newpkg);
                AlpmPackage::m_change_statuses.remove(e->newpkg);
                break;
            case ALPM_PACKAGE_REINSTALL:
            case ALPM_PACKAGE_REMOVE:
                AlpmPackage::m_change_statuses.remove(e->oldpkg);
                break;
            }
            break;
        }
        case ALPM_EVENT_INTEGRITY_START:
        {
            QString message = QObject::tr("Checking package integrity...");
            p_alpm->emit_event("checking_integrity",Q_ARG(QString,message));
            p_alpm->emit_information(message);
            break;
        }
        case ALPM_EVENT_KEYRING_START:
        {
            QString message = QObject::tr("Checking keyring...");
            p_alpm->emit_event("checking_keyring",Q_ARG(QString,message));
            p_alpm->emit_information(message);
            break;
        }
        case ALPM_EVENT_KEY_DOWNLOAD_START:
        {
            QString message = QObject::tr("Downloading required keys...");
            p_alpm->emit_event("checking_key_download",Q_ARG(QString,message));
            p_alpm->emit_information(message);
            break;
        }
        case ALPM_EVENT_LOAD_START:
        {
            QString message = QObject::tr("Loading package files...");
            p_alpm->emit_event("loading_pkg_files",Q_ARG(QString,message));
            p_alpm->emit_information(message);
            break;
        }
        case ALPM_EVENT_SCRIPTLET_INFO:
        {
            if (prev_event_type != ALPM_EVENT_SCRIPTLET_INFO) p_alpm->emit_event("starting_scriplet",Q_ARG(QString,QObject::tr("Executing an internal scriplet...")));
            p_alpm->emit_information(QString::fromLocal8Bit((const char *)event->scriptlet_info.line));
            break;
        }
        case ALPM_EVENT_PKG_RETRIEVE_START:
            m_download_errs.clear();
            p_alpm->emit_information(QObject::tr("Retrieving packages..."));
            p_alpm->emit_event("downloads_starting");
            break;
        case ALPM_EVENT_DISKSPACE_START:
        {
            QString message = QObject::tr("Checking available disk space...");
            p_alpm->emit_event("checking_diskspace",Q_ARG(QString,message));
            p_alpm->emit_information(message);
            break;
        }
        case ALPM_EVENT_OPTDEP_REMOVAL:
        {
            alpm_event_optdep_removal_t *e = &event->optdep_removal;
            char * dep_string = alpm_dep_compute_string(e->optdep);
            p_alpm->emit_information(QObject::tr("%1 optionally requires %2").arg(QString::fromLocal8Bit((const char *)alpm_pkg_get_name(e->pkg))).
                                                                                  arg(QString::fromLocal8Bit((const char *)dep_string)));
            free(dep_string);
            break;
        }
        case ALPM_EVENT_DATABASE_MISSING:
            p_alpm->emit_information(QObject::tr("Database file for '%1' does not exist, you need to update your databases!").
                                                                                  arg(QString::fromLocal8Bit((const char *)event->database_missing.dbname)));
            break;
        case ALPM_EVENT_PACNEW_CREATED:
        {
            alpm_event_pacnew_created_t *e = &event->pacnew_created;
            p_alpm->emit_information(QObject::tr("%1 installed as %12.pacnew").arg(QString::fromLocal8Bit((const char *)e->file)));
            break;
        }
        case ALPM_EVENT_PACSAVE_CREATED:
        {
            alpm_event_pacsave_created_t *e = &event->pacsave_created;
            p_alpm->emit_information(QObject::tr("%1 saved as %1.pacsave").arg(QString::fromLocal8Bit((const char *)e->file)));
            break;
        }
        /* all the simple done events, with fallthrough for each */
        case ALPM_EVENT_FILECONFLICTS_DONE:
            p_alpm->emit_event("file_conflicts_checked");
            break;
        case ALPM_EVENT_CHECKDEPS_DONE:
            p_alpm->emit_event("pkg_deps_checked");
            break;
        case ALPM_EVENT_RESOLVEDEPS_DONE:
            p_alpm->emit_event("pkg_deps_resolved");
            break;
        case ALPM_EVENT_INTERCONFLICTS_DONE:
            p_alpm->emit_event("internal_conflicts_checked");
            break;
        case ALPM_EVENT_TRANSACTION_DONE:
            p_alpm->emit_event("transaction_completed");
            break;
        case ALPM_EVENT_INTEGRITY_DONE:
            p_alpm->emit_event("integrity_checked");
            break;
        case ALPM_EVENT_KEYRING_DONE:
            p_alpm->emit_event("keyring_checked");
            break;
        case ALPM_EVENT_KEY_DOWNLOAD_DONE:
            p_alpm->emit_event("key_download_checked");
            break;
        case ALPM_EVENT_LOAD_DONE:
            p_alpm->emit_event("pkg_files_loaded");
            break;
        case ALPM_EVENT_DISKSPACE_DONE:
            p_alpm->emit_event("diskspace_checked");
            break;
        case ALPM_EVENT_PKG_RETRIEVE_DONE:
            p_alpm->emit_information(QObject::tr("Packages have been retrieved successfully!"));
            p_alpm->emit_event("downloads_completed");
            break;
        case ALPM_EVENT_PKG_RETRIEVE_FAILED:
            m_download_errs.removeDuplicates();
            p_alpm->emit_information(QObject::tr("Packages have not been retrieved :("));
            p_alpm->emit_event("downloads_failed");
            break;
        case ALPM_EVENT_HOOK_DONE:
            p_alpm->emit_event("all_hooks_completed");
            break;
        case ALPM_EVENT_HOOK_RUN_DONE:
        {
            alpm_event_hook_run_t *e = &event->hook_run;
            p_alpm->emit_event("hook_completed",Q_ARG(QString,e->desc?QString::fromLocal8Bit(e->desc):QString::fromLocal8Bit(e->name)));
            break;
        }
    }
    prev_event_type = event->type;
}

bool Alpm::isValid(bool change_errno) const {
    bool ret = (m_alpm_handle != NULL);
    if (change_errno && !ret) ((Alpm *)this)->m_alpm_errno=ALPM_IS_NOT_OPEN;
    return ret;
}

const QString Alpm::version() {
    const char * ver = alpm_version();
    if (ver == NULL) return QString();
    return QString::fromLocal8Bit(ver);
}

QString Alpm::arch() const {
    if (!isValid(true)) return QString();

    const char * arch = (const char *)alpm_option_get_architectures(m_alpm_handle)->data;
    if (arch == NULL) return QString();
    return QString::fromLocal8Bit(arch);
}

ThreadRun::RC Alpm::lastMethodRC() const {
    int error_id = ALPM_ERR_OK;
    lastError(&error_id);
    return (error_id == USER_REFUSAL)?ThreadRun::TERMINATED:((error_id != ALPM_ERR_OK)?ThreadRun::BAD:ThreadRun::OK);
}

QString Alpm::lastError(int * error_id) const {
    if (!isValid()) {
        if (error_id != NULL) *error_id = ALPM_ERR_HANDLE_NULL;
        return QString::fromLocal8Bit(alpm_strerror(ALPM_ERR_HANDLE_NULL));
    }

    if (m_alpm_errno == OK_CODE_SUPPRESS_ALPMS) {
        if (error_id != NULL) *error_id = ALPM_ERR_OK;
        return QString();
    }

    if (m_alpm_errno == ALPM_ERR_EXTERNAL_DOWNLOAD && !m_download_errs.isEmpty()) {
        if (error_id != NULL) *error_id = ALPM_ERR_EXTERNAL_DOWNLOAD;
        return m_download_errs.join("\n");
    }

    if (m_alpm_errno < ALPM_ERR_OK) {
        if (error_id != NULL) *error_id = m_alpm_errno;
        switch (m_alpm_errno) {
        case PKG_LIST_IS_EMPTY:
            return tr("The input package list is empty!");
        case ALPM_INSTANCE_ALREADY_CREATED:
            return tr("Alpm class has been already initialized!");
        case ALPM_IS_NOT_OPEN:
            return tr("Alpm class is open yet!");
        case PKG_IS_NOT_INITED:
            return tr("The package class has not been properly intialized!");
        case REASON_WRONG_DB:
            return tr("The reason can be changed only for local packages!");
        case CANNOT_GET_ROOT:
            return tr("Cannot obtain the root rights!");
        case ALPM_CONFIG_FAILED:
            return tr("Cannot load config file!");
        case ALPM_LINK_LOCAL_DB_FAILED:
            return tr("Cannot create the link for local's db!");
        case ALPM_HANDLE_FAILED:
            return tr("Cannot create Alpm handle from config handle!");
        case THREAD_IS_STILL_RUNNING:
            return tr("The function in the thread is still running!");
        case CANNOT_LOAD_CONFIG:
            return m_config.lastError();
        case NOTHING_DOWNLOAD:
            return tr("Nothing to download!!!");
        case USER_REFUSAL:
            return tr("Stopped because of user refusal!!!");
        default:
            break;
        }
    }
    else if (m_alpm_errno > ALPM_ERR_OK) return QString::fromLocal8Bit(alpm_strerror((alpm_errno_t)m_alpm_errno));

    if (alpm_errno(m_alpm_handle) == ALPM_ERR_OK) {
        if (error_id != NULL) *error_id = ALPM_ERR_OK;
        return QString();
    }

    return QString::fromLocal8Bit(alpm_strerror(alpm_errno(m_alpm_handle)));
}

QList<AlpmDB> Alpm::allSyncDBs() const {
    return m_syncDBs;
}

AlpmDB Alpm::localDB() const {
    return m_localDB;
}

bool Alpm::pkg_equal_cmp(const AlpmPackage & pkg1, const AlpmPackage & pkg2) {
    int ret = pkg1.name().compare(pkg2.name());
    if (ret) return false;
    ret = AlpmPackage::pkg_vercmp(pkg1.version(),pkg2.version());
    if (ret) return false;
    return !pkg1.repo().compare(pkg2.repo());
}

bool Alpm::pkg_less_cmp(const AlpmPackage & pkg1, const AlpmPackage & pkg2) {
    int ret;
    if ((ret = pkg1.name().compare(pkg2.name())) != 0) return (ret < 0);
    if ((ret = AlpmPackage::pkg_vercmp(pkg1.version(),pkg2.version())) != 0) return (ret < 0);
    return (pkg1.repo().compare(pkg2.repo()) < 0);
}

bool Alpm::sort_cmp(const AlpmPackage & item1,const AlpmPackage & item2) {
    int ret;
    if ((ret = item1.name().compare(item2.name())) != 0) return (ret < 0);
    return (AlpmPackage::pkg_vercmp(item1.version(),item2.version()) < 0);
}

bool Alpm::sort_equal_cmp(const AlpmPackage & item1,const AlpmPackage & item2) {
    if (item1.name().compare(item2.name())) return false;
    return !AlpmPackage::pkg_vercmp(item1.version(),item2.version());
}

void Alpm::query_packages_portion(QList<AlpmPackage> & pkgs,int startindex,int lastindex) const {
    if (pkgs[startindex].repo() == "local") {
        int count = 0;
        for (int i=startindex+1;i<=lastindex;i++) {
            if (sort_equal_cmp(pkgs[i],pkgs[startindex])) count++;
        }
        if (count >= 1) pkgs[startindex] = AlpmPackage();
    }
}

QList<AlpmPackage> Alpm::query_packages(const QString & name,AlpmPackage::SearchFieldType fieldType,AlpmPackage::PackageFilter filter,const QString & group,const QString & repo) const {
    QList<AlpmPackage> m_packages;

    for (AlpmDB & db: allSyncDBs()) {
        if (!repo.isEmpty() && repo != db.name() && repo != "local") continue;
        m_packages += db.packages(name,fieldType,filter,group);
    }

    if (repo.isEmpty() || repo == "local") m_packages += localDB().packages(name,fieldType,filter,group);

    std::sort(m_packages.begin(),m_packages.end(),pkgLessThan);

    int startindex = -1;
    int lastindex = -1;
    for (int j=0;j<m_packages.count();j++) {
        if (startindex == -1) {
            startindex = lastindex = 0;
            continue;
        }
        if (sort_cmp(m_packages[j-1],m_packages[j])) {
            query_packages_portion(m_packages,startindex,lastindex);
            startindex = lastindex = j;
            continue;
        }
        else lastindex = j;
    }
    if (startindex >= 0 && lastindex >= 0) query_packages_portion(m_packages,startindex,lastindex);

    m_packages.erase(std::remove_if(m_packages.begin(),m_packages.end(),[](const AlpmPackage & pkg){return !pkg.isValid();}),m_packages.end());
    if (repo == "local") m_packages.erase(std::remove_if(m_packages.begin(),m_packages.end(),[](const AlpmPackage & pkg){return (pkg.repo() != "local");}),m_packages.end());

    return m_packages;
}

const QStringList Alpm::dirContents(const QDir & dir,const QString & nameFilter) {
    QStringList ret;
    QDirIterator it(dir.path(),QStringList() << nameFilter);
    while (it.hasNext()) {
        ret << it.next();
    }
    return ret;
}

bool Alpm::queryPackages(const QString & name,AlpmPackage::SearchFieldType fieldType,AlpmPackage::PackageFilter filter,const QString & group,const QString & repo) {
    if (!isValid(true)) return false;

    for (const QString & filename: dirContents(dbDirPath()+QDir::separator()+QString::fromLatin1("sync"),"*.part")) QFile(filename).remove();

    return (run<QList<AlpmPackage> >(this,&Alpm::query_packages,name,fieldType,filter,group,repo) == ThreadRun::OK);
}

ThreadRun::RC Alpm::updateDBs(bool force) {
    if (!isValid(true)) return ThreadRun::BAD;

    if (!reopen()) {
        emit_information(lastError());
        emit_error(lastError());
        return ThreadRun::BAD;
    }

    m_download_errs.clear();

    return run_void(this,&Alpm::update_dbs,force);
}

void Alpm::update_dbs(bool force) {
    m_percent = -1;
    m_alpm_errno = ALPM_ERR_OK;

    QList<AlpmDB> dbs = allSyncDBs();
    for (int i=0;i<dbs.count();i++) {
        emit_event("download_db_start",Q_ARG(QString,dbs[i].name()));
        emit_information(QObject::tr("Updating %1 db...").arg(dbs[i].name()));
        if (!dbs[i].update(force)) {
            emit_information(lastError());
            emit_error(lastError());
            return;
        }
    }
    AlpmPackage::m_change_statuses.clear();
    recreatedbs();

    m_alpm_errno = ALPM_ERR_OK;
}

QString Alpm::download_package(const QString & download_url) const {
    QString cache_dir = cacheDirs().at(0);
    if (Alpm::operation_fetch_fn(NULL,download_url,cache_dir,true) == -1) return QString();
    return cache_dir+QDir::separator()+QUrl(download_url).fileName();
}

QString Alpm::download_package(const AlpmPackage & pkg) const {
    QString ret;
    for (const QLatin1String & remote_loc: pkg.remoteLocations()) {
        if (remote_loc.isEmpty()) continue;
        ret = download_package(remote_loc+QDir::separator()+pkg.fileName());
        if (!ret.isEmpty()) break;
    }
    return ret;
}

QStringList Alpm::download_packages(const QList<AlpmPackage> & pkgs) {
    QStringList downloaded_paths;
    if (!isValid(true)) return downloaded_paths;

    m_alpm_errno = ALPM_ERR_OK;

    m_percent = -1;

    qint64 download_size = 0;
    alpm_pkg_t * handle;
    int i;
    for (i=0;i<pkgs.count();i++) {
        if (pkgs.at(i).remoteLocations().isEmpty()) continue;
        handle = pkgs.at(i).handle();
        if (handle == NULL) continue;
        download_size += alpm_pkg_download_size(handle);
    }

    if (download_size <= 0) {
        m_alpm_errno = NOTHING_DOWNLOAD;
        emit_error(lastError());
        emit_event("downloads_failed");
        return downloaded_paths;
    }

    if (download_size > 0) emit_event("full_download_size_found",Q_ARG(qint64,download_size));

    QString filename;
    QString out_path;
    for (i=0;i<pkgs.count();i++) {
        filename = pkgs.at(i).fileName();
        emit_event("download_start",Q_ARG(QString,filename));
        out_path = download_package(pkgs.at(i));
        if (!out_path.isEmpty()) downloaded_paths.append(out_path);
        else {
            emit_event("downloads_failed");
            return downloaded_paths;
        }
    }

    emit_event("downloads_completed");

    return downloaded_paths;
}

ThreadRun::RC Alpm::downloadPackages(const QList<AlpmPackage> & pkgs) {
    if (!isValid(true) || pkgs.count() <= 0) return ThreadRun::BAD;

    m_download_errs.clear();

    return run<QStringList>(this,&Alpm::download_packages,pkgs);
}

QList<AlpmPackage> Alpm::updates() const {
    QList<AlpmPackage> ret;

    for (AlpmPackage & pkg: query_packages()) {
        if (pkg.isUpdate()) ret.append(pkg);
    }

    std::sort(ret.begin(),ret.end(),sort_cmp);
    ret.erase(std::unique(ret.begin(),ret.end(),sort_equal_cmp),ret.end());

    return ret;
}

bool Alpm::do_process_targets(bool remove) {
    QStringList install_targets;
    QStringList remove_targets;

    AlpmList<alpm_pkg_t> list(alpm_trans_get_add(m_alpm_handle),AlpmList<alpm_pkg_t>::ignorefree);
    AlpmList<alpm_pkg_t> rlist(alpm_trans_get_remove(m_alpm_handle),AlpmList<alpm_pkg_t>::ignorefree);

    qint64 dl_size = 0;
    qint64 install_size = 0;
    qint64 remove_size = 0;
    char * db_name;

    if (!remove && list.count() > 0) {
        do {
            alpm_pkg_t *pkg = list.valuePtr();
            dl_size += alpm_pkg_download_size(pkg);
            install_size += alpm_pkg_get_isize(pkg);
            install_targets.append(QString("%3/%1=%2").arg(QString::fromLocal8Bit(alpm_pkg_get_name(pkg))).arg(QString::fromLocal8Bit(alpm_pkg_get_version(pkg))).arg(QString::fromLocal8Bit((db_name = (char *)alpm_db_get_name(alpm_pkg_get_db(pkg)))?db_name:"local")));
        } while(list.goNext());
    }
    list.detach();

    if (rlist.count() > 0) {
        do {
            alpm_pkg_t *pkg = rlist.valuePtr();
            remove_size += alpm_pkg_get_isize(pkg);
            remove_targets.append(QString("%3/%1=%2").arg(QString::fromLocal8Bit(alpm_pkg_get_name(pkg))).arg(QString::fromLocal8Bit(alpm_pkg_get_version(pkg))).arg(QString::fromLocal8Bit((db_name = (char *)alpm_db_get_name(alpm_pkg_get_db(pkg)))?db_name:"local")));
        } while(rlist.goNext());
    }
    rlist.detach();

    if (!remove && install_targets.count() <= 0) return false;
    if (remove && remove_targets.count() <= 0) return false;

    bool ok;
    if (remove) emit_remove_packages_confirmation(remove_targets,remove_size,&ok);
    else emit_install_packages_confirmation(install_targets,remove_targets,dl_size,install_size,remove_size,&ok);
    return ok;
}

class HandleReleaser {
public:
    HandleReleaser(alpm_handle_t * handle) {
        m_handle = handle;
    }
    ~HandleReleaser() {
        ::alpm_trans_release(m_handle);
    }
private:
    alpm_handle_t * m_handle;
};

bool Alpm::only_pkg_name_cmp(const AlpmPackage & item1, const AlpmPackage & item2) {
    return (item1.name().compare(item2.name()) < 0);
}

void Alpm::sync_sysupgrade_portion(QList<AlpmPackage> & add_pkgs,int startindex,int lastindex) {
    if (startindex == lastindex) return;

    int _startindex = -1;
    int _lastindex = -1;
    int i;
    for (i=startindex;i<=lastindex;i++) {
        if (_startindex == -1) {
            _startindex = _lastindex = i;
            continue;
        }
        if (sort_cmp(add_pkgs[i-1],add_pkgs[i])) {
            _startindex = _lastindex = i;
            continue;
        }
        else _lastindex = i;
    }
    if (_startindex == -1 || _lastindex == -1) return;

    for (i=(_startindex-1);i>=startindex;i--) {
        add_pkgs[i] = AlpmPackage();
    }

    if (_startindex == _lastindex) return;

    int sel_index = _startindex;
    alpm_question_select_provider_t provider_question;
    provider_question.type = ALPM_QUESTION_SELECT_PROVIDER;
    provider_question.use_index = 0;
    alpm_list_t *providers = NULL;
    for (i=_startindex;i<=_lastindex;i++) {
        providers = alpm_list_add(providers,add_pkgs[i].handle());
    }
    provider_question.providers = providers;
    alpm_depend_t alpm_dep = depend_t(AlpmPackage::Dependence(add_pkgs[_startindex]));
    provider_question.depend = &alpm_dep;
    operation_question_fn(NULL,(alpm_question_t *)&provider_question);
    sel_index = _startindex + provider_question.use_index;
    free(alpm_dep.name);
    free(alpm_dep.desc);
    free(alpm_dep.version);
    alpm_list_free(providers);

    for (i=_lastindex;i>=_startindex;i--) {
        if (i == sel_index) continue;
        add_pkgs[i] = AlpmPackage();
    }
}

void Alpm::sync_sysupgrade(int m_install_flags) {
    m_alpm_errno = ALPM_ERR_OK;

    QList<AlpmPackage> add_pkgs;
    QList<AlpmPackage> remove_pkgs;
    int sel_index = 0;
    alpm_question_replace_t question;
    question.type = ALPM_QUESTION_REPLACE_PKG;
    alpm_question_select_provider_t provider_question;
    provider_question.type = ALPM_QUESTION_SELECT_PROVIDER;
    QList<AlpmPackage> new_pkgs;
    int j;
    QMapIterator<AlpmPackage,QList<AlpmPackage> > i(m_replaces);
    while (i.hasNext()) {
        i.next();
        new_pkgs = i.value();
        std::sort(new_pkgs.begin(),new_pkgs.end(),pkg_less_cmp);
        new_pkgs.erase(std::unique(new_pkgs.begin(),new_pkgs.end(),pkg_equal_cmp),new_pkgs.end());

        sel_index = 0;
        provider_question.use_index = 0;
        if (new_pkgs.count() > 1) {
            alpm_list_t *providers = NULL;
            for (j=0;j<new_pkgs.count();j++) {
                providers = alpm_list_add(providers,new_pkgs[j].handle());
            }
            provider_question.providers = providers;
            AlpmPackage::Dependence dep(i.key());
            alpm_depend_t alpm_dep = depend_t(dep);
            provider_question.depend = &alpm_dep;
            operation_question_fn(NULL,(alpm_question_t *)&provider_question);
            sel_index = provider_question.use_index;
            free(alpm_dep.name);
            free(alpm_dep.desc);
            free(alpm_dep.version);
            alpm_list_free(providers);
        }

        question.replace = 0;
        question.oldpkg = i.key().handle(),
        question.newpkg = new_pkgs[sel_index].handle(),
        question.newdb = alpm_pkg_get_db(question.newpkg);
        operation_question_fn(NULL,(alpm_question_t *)&question);
        if (question.replace) {
            add_pkgs.append(new_pkgs[sel_index]);
            remove_pkgs.append(i.key());
        }
    }

    add_pkgs = updates();
    std::sort(remove_pkgs.begin(),remove_pkgs.end(),sort_cmp);
    std::sort(add_pkgs.begin(),add_pkgs.end(),sort_cmp);

    QList<AlpmPackage>::const_iterator it;
    for (j=0;j<remove_pkgs.count();j++) {
        it = binary_search_ex(add_pkgs.begin(),add_pkgs.end(),remove_pkgs[j],only_pkg_name_cmp);
        if (it == add_pkgs.end()) continue;
        add_pkgs[it-add_pkgs.begin()] = AlpmPackage();
        j--;
    }

    add_pkgs.erase(std::remove_if(add_pkgs.begin(),add_pkgs.end(),[](const AlpmPackage & pkg){return !pkg.isValid();}),add_pkgs.end());
    std::sort(add_pkgs.begin(),add_pkgs.end(),pkg_less_cmp);
    add_pkgs.erase(std::unique(add_pkgs.begin(),add_pkgs.end(),pkg_equal_cmp),add_pkgs.end());

    int startindex = -1;
    int lastindex = -1;
    for (j=0;j<add_pkgs.count();j++) {
        if (startindex == -1) {
            startindex = lastindex = 0;
            continue;
        }
        if (only_pkg_name_cmp(add_pkgs[j-1],add_pkgs[j])) {
            sync_sysupgrade_portion(add_pkgs,startindex,lastindex);
            startindex = lastindex = j;
            continue;
        }
        else lastindex = j;
    }
    if (startindex >= 0 && lastindex >= 0) sync_sysupgrade_portion(add_pkgs,startindex,lastindex);

    add_pkgs.erase(std::remove_if(add_pkgs.begin(),add_pkgs.end(),[](const AlpmPackage & pkg){return !pkg.isValid();}),add_pkgs.end());

    QStringList syncfirst = m_config.syncFirstPkgs();
    if (!syncfirst.isEmpty() && !add_pkgs.isEmpty()) {
        QList<AlpmPackage> syncfirst_pkgs;
        for (int i=(add_pkgs.count()-1);i>=0;i--) {
            if (syncfirst.contains(add_pkgs[i].name())) syncfirst_pkgs.append(add_pkgs.takeAt(i));
        }
        if (!syncfirst_pkgs.isEmpty()) {
            ::alpm_trans_release(m_alpm_handle);
            install_packages(syncfirst_pkgs,0,QList<AlpmPackage>());
            if (alpm_trans_init(m_alpm_handle,m_install_flags)) {
                emit_information(lastError());
                emit_error(lastError());
                return;
            }
        }
    }

    for (j=0;j<remove_pkgs.count();j++) {
        if (alpm_remove_pkg(m_alpm_handle,remove_pkgs[j].handle())) {
            QString err = QString("%1: %2").arg(remove_pkgs[j].name()).arg(lastError());
            emit_information(err);
            emit_error(err);
            return;
        }
    }

    for (j=0;j<add_pkgs.count();j++) {
        if (alpm_add_pkg(m_alpm_handle,add_pkgs[j].handle()) && alpm_errno(m_alpm_handle) != ALPM_ERR_TRANS_DUP_TARGET) {
            QString err = QString("%1: %2").arg(add_pkgs[j].name()).arg(lastError());
            emit_information(err);
            emit_error(err);
            return;
        }
    }
}

void Alpm::install_packages(const QList<AlpmPackage> & m_pkgs,int m_install_flags,const QList<AlpmPackage> & forcedpkgs) {
    m_alpm_errno = ALPM_ERR_OK;
    m_percent = -1;
    prev_event_type = -1;
    QString err;

    HandleReleaser handle_releaser(m_alpm_handle);
    OverwriteHandler handler(forcedpkgs);

    if (alpm_trans_init(m_alpm_handle,m_install_flags)) {
        emit_information(lastError());
        emit_error(lastError());
        return;
    }

    if (m_pkgs.count() <= 0) {
        emit_information(tr("Starting full system upgrade..."),true);
        alpm_logaction(m_alpm_handle,LOGPREFIX,"starting full system upgrade\n");
        sync_sysupgrade(m_install_flags);
        int error_id;
        lastError(&error_id);
        if(error_id != ALPM_ERR_OK) {
            emit_information(lastError());
            emit_error(lastError());
            return;
        }
    }
    else {
        alpm_pkg_t * handle;
        for (int i=0;i<m_pkgs.count();i++) {
            handle = m_pkgs[i].handle();
            if (handle == NULL) {
                emit_information(tr("Skipping target: %1").arg(m_pkgs[i].toString()));
                continue;
            }
            int ret = alpm_add_pkg(m_alpm_handle,handle);
            if (ret) {
                if(alpm_errno(m_alpm_handle) == ALPM_ERR_TRANS_DUP_TARGET) {
                    emit_information(tr("Skipping target: %1").arg(m_pkgs[i].toString()));
                    continue;
                }
                err = tr("%1: %2").arg(m_pkgs[i].name()).arg(lastError());
                emit_information(err);
                emit_error(err);
                return;
            }
        }
    }

    alpm_list_t *data = NULL;
    char * tmpstr;
    if(alpm_trans_prepare(m_alpm_handle, &data)) {
        switch (alpm_errno(m_alpm_handle)) {
        case ALPM_ERR_PKG_INVALID_ARCH:
        {
            AlpmList<char> list(data);
            do {
                if (list.isEmpty()) break;
                err = tr("Package %1 does not have a valid architecture").arg(QString::fromLocal8Bit(list.valuePtr()));
                emit_information(err);
                emit_error(err);
            } while(list.goNext());
            data = NULL;
            break;
        }
        case ALPM_ERR_UNSATISFIED_DEPS:
        {
            AlpmList<alpm_depmissing_t> list(data,alpm_depmissing_free);
            do {
                if (list.isEmpty()) break;
                tmpstr = alpm_dep_compute_string(list.valuePtr()->depend);
                QString depstring = QString::fromLocal8Bit(tmpstr);
                free(tmpstr);
                alpm_list_t *trans_add = alpm_trans_get_add(m_alpm_handle);
                alpm_pkg_t *pkg;
                if(list.valuePtr()->causingpkg == NULL) {
                    err = tr("Unable to satisfy dependency '%1' required by %2").arg(depstring).arg(QString::fromLocal8Bit(list.valuePtr()->target));
                    emit_information(err);
                    emit_error(err);
                } else if((pkg = alpm_pkg_find(trans_add,list.valuePtr()->causingpkg))) {
                    err = tr("Installing %1 (%2) breaks dependency '%3' required by %4").arg(QString::fromLocal8Bit(list.valuePtr()->causingpkg)).arg(QString::fromLocal8Bit(alpm_pkg_get_version(pkg))).arg(depstring).arg(QString::fromLocal8Bit(list.valuePtr()->target));
                    emit_information(err);
                    emit_error(err);
                } else {
                    err = tr("Removing %1 breaks dependency '%2' required by %3").arg(QString::fromLocal8Bit(list.valuePtr()->causingpkg)).arg(depstring).arg(QString::fromLocal8Bit(list.valuePtr()->target));
                    emit_information(err);
                    emit_error(err);
                }
            } while(list.goNext());
            data = NULL;
            break;
        }
        case ALPM_ERR_CONFLICTING_DEPS:
        {
            AlpmList<alpm_conflict_t> list(data,alpm_conflict_free);
            do {
                if (list.isEmpty()) break;
                if(list.valuePtr()->reason->mod == ALPM_DEP_MOD_ANY) {
                    err = tr("%1 and %2 are in conflict").arg(QString::fromLocal8Bit(list.valuePtr()->package1)).arg(QString::fromLocal8Bit(list.valuePtr()->package2));
                    emit_information(err);
                    emit_error(err);
                } else {
                    char *reason = alpm_dep_compute_string(list.valuePtr()->reason);
                    err = tr("%1 and %2 are in conflict (%3)").arg(QString::fromLocal8Bit(list.valuePtr()->package1)).arg(QString::fromLocal8Bit(list.valuePtr()->package2)).arg(QString::fromLocal8Bit(reason));
                    emit_information(err);
                    emit_error(err);
                    free(reason);
                }
            } while(list.goNext());
            data = NULL;
            break;
        }
        default:
            emit_information(lastError());
            emit_error(lastError());
            FREELIST(data);
            break;
        }
        return;
    }

    FREELIST(data);
    if (alpm_list_count(alpm_trans_get_add(m_alpm_handle)) <= 0) {
        emit_information(tr("No packages were upgraded because there is nothing to install."),true);
        m_alpm_errno = OK_CODE_SUPPRESS_ALPMS;
        return;
    }

    if (!do_process_targets(false)) {
        emit_information(tr("No packages were upgraded because of user's refusal."),true);
        m_alpm_errno = USER_REFUSAL;
        return;
    }

    if(alpm_trans_commit(m_alpm_handle, &data)) {
        emit_information(tr("Failed to commit transaction (%1)").arg(lastError()));
        switch(alpm_errno(m_alpm_handle)) {
        case ALPM_ERR_FILE_CONFLICTS:
        {
            AlpmList<alpm_fileconflict_t> list(data,alpm_fileconflict_free);
            do {
                if (list.isEmpty()) break;
                alpm_fileconflict_t *conflict = list.valuePtr();
                switch(conflict->type) {
                    case ALPM_FILECONFLICT_TARGET:
                        err = tr("%1 exists in both '%2' and '%3'").arg(QString::fromLocal8Bit(conflict->file)).arg(QString::fromLocal8Bit(conflict->target)).arg(QString::fromLocal8Bit(conflict->ctarget));
                        emit_information(err);
                        emit_error(err);
                        break;
                    case ALPM_FILECONFLICT_FILESYSTEM:
                        if(conflict->ctarget[0]) {
                            err = tr("%1: %2 exists in filesystem (owned by %3)").arg(QString::fromLocal8Bit(conflict->target)).arg(QString::fromLocal8Bit(conflict->file)).arg(QString::fromLocal8Bit(conflict->ctarget));
                            emit_information(err);
                            emit_error(err);
                        }
                        else {
                            err = tr("%1: %2 exists in filesystem").arg(QString::fromLocal8Bit(conflict->target)).arg(QString::fromLocal8Bit(conflict->file));
                            emit_information(err);
                            emit_error(err);
                        }
                        break;
                }
            } while(list.goNext());
            data = NULL;
            break;
        }
        case ALPM_ERR_PKG_INVALID:
        case ALPM_ERR_PKG_INVALID_CHECKSUM:
        case ALPM_ERR_PKG_INVALID_SIG:
        {
            AlpmList<char> list(data);
            do {
                if (list.isEmpty()) break;
                err = tr("%1 is invalid or corrupted").arg(QString::fromLocal8Bit(list.valuePtr()));
                emit_information(err);
                emit_error(err);
            } while(list.goNext());
            data = NULL;
            break;
        }
        default:
            emit_information(lastError());
            emit_error(lastError());
            FREELIST(data);
            break;
        }
        emit_information(tr("Errors occurred, no packages were installed."),true);
        return;
    }

    emit_information(tr("No errors occurred, the packages were successfully installed."),true);
    FREELIST(data);

    m_alpm_errno = ALPM_ERR_OK;
}

int Alpm::fnmatch_cmp(const void *pattern, const void *string) {
    return fnmatch((const char *)pattern,(const char *)string,0);
}

void Alpm::remove_packages(const QList<AlpmPackage> & m_pkgs,bool remove_cascade) {
    m_percent = -1;
    prev_event_type = -1;
    m_alpm_errno = ALPM_ERR_OK;
    QString err;

    HandleReleaser handle_releaser(m_alpm_handle);

    if (alpm_trans_init(m_alpm_handle,remove_cascade?ALPM_TRANS_FLAG_RECURSE|ALPM_TRANS_FLAG_CASCADE:0)) {
        emit_information(lastError());
        emit_error(lastError());
        return;
    }

    for (int i=0;i<m_pkgs.count();i++) {
        AlpmPackage pkg = localDB().findByPackageName(m_pkgs[i].name());
        if (!pkg.isValid()) continue;
        if (alpm_remove_pkg(m_alpm_handle,pkg.handle())) {
            if(alpm_errno(m_alpm_handle) == ALPM_ERR_TRANS_DUP_TARGET) {
                emit_information(tr("Skipping target: %1").arg(m_pkgs[i].name()));
                continue;
            }
            err = QString("%1: %2").arg(m_pkgs[i].name()).arg(lastError());
            emit_information(err);
            emit_error(err);
            return;
        }
    }

    alpm_list_t *data = NULL;
    char * tmpstr;
    if(alpm_trans_prepare(m_alpm_handle, &data)) {
        emit_information(tr("Failed to prepare transaction (%1)").arg(lastError()));
        switch(alpm_errno(m_alpm_handle)) {
        case ALPM_ERR_UNSATISFIED_DEPS:
        {
            AlpmList<alpm_depmissing_t> list(data,alpm_depmissing_free);
            do {
                if (list.isEmpty()) break;
                tmpstr = alpm_dep_compute_string(list.valuePtr()->depend);
                QString depstring = QString::fromLocal8Bit(tmpstr);
                free(tmpstr);
                err = tr("%1: removing %2 breaks dependency '%3'").arg(QString::fromLocal8Bit(list.valuePtr()->target)).arg(QString::fromLocal8Bit(list.valuePtr()->causingpkg)).arg(depstring);
                emit_information(err);
                emit_error(err);
            } while(list.goNext());
            data = NULL;
            break;
        }
        default:
            emit_information(lastError());
            emit_error(lastError());
            FREELIST(data);
            break;
        }
        return;
    }

    FREELIST(data);

    bool ok;
    AlpmList<alpm_pkg_t> list(alpm_trans_get_remove(m_alpm_handle),AlpmList<alpm_pkg_t>::ignorefree);
    do {
        if (list.isEmpty()) break;
        alpm_pkg_t *pkg = list.valuePtr();
        QString name = QString::fromLocal8Bit(alpm_pkg_get_name(pkg));
        if(alpm_list_find(m_config.holdpkgs,alpm_pkg_get_name(pkg),fnmatch_cmp)) {
            emit_question(tr("%1 is designated as a HoldPkg, Do you want to continue?").arg(name),&ok);
            if (!ok) {
                list.detach();
                emit_error(tr("User rejected the removal of %1 package!").arg(name));
                m_alpm_errno = USER_REFUSAL;
                return;
            }
        }
    } while(list.goNext());
    list.detach();

    if (!do_process_targets(true)) {
        emit_information(tr("No packages were removed because of user's refusal."),true);
        m_alpm_errno = USER_REFUSAL;
        return;
    }

    if(alpm_trans_commit(m_alpm_handle, &data)) {
        err = tr("Failed to commit transaction (%1)").arg(lastError());
        emit_information(err);
        emit_error(err);
        FREELIST(data);
        return;
    }
    FREELIST(data);

    emit_information(tr("No errors occurred, the packages were successfully removed."),true);
    m_alpm_errno = ALPM_ERR_OK;
}

ThreadRun::RC Alpm::installPackages(const QList<AlpmPackage> & pkgs,bool asdeps,const QList<AlpmPackage> & forcedpkgs) {
    if (!isValid(true)) return ThreadRun::BAD;

    return run_void(this,&Alpm::install_packages,pkgs,asdeps?ALPM_TRANS_FLAG_ALLDEPS:0,forcedpkgs);
}

ThreadRun::RC Alpm::removePackages(const QList<AlpmPackage> & pkgs,bool cascade) {
    if (!isValid(true)) return ThreadRun::BAD;

    return run_void(this,&Alpm::remove_packages,pkgs,cascade);
}

bool Alpm::isLocked() const {
    if (!isValid(true)) return true;

    return QFileInfo(lockFilePath()).exists();
}

bool Alpm::removeLockFile() {
    if (!isLocked()) return true;

    QFile(lockFilePath()).remove();
    return !isLocked();
}

QString Alpm::lockFilePath() const {
    return QString::fromLocal8Bit(alpm_option_get_lockfile(m_alpm_handle));
}

QString Alpm::dbDirPath() const {
    QString ret = QString::fromLocal8Bit(alpm_option_get_dbpath(m_alpm_handle));
    return (ret.endsWith(QDir::separator()))?ret.mid(0,ret.length()-1):ret;
}

QStringList Alpm::cacheDirs() const {
    if (!isValid(true)) return QStringList();

    QStringList ret;
    AlpmList<char> dirs(alpm_option_get_cachedirs(m_alpm_handle),AlpmList<char>::ignorefree);
    if (dirs.count() <= 0) return ret;

    do {
        ret.append(QString::fromLocal8Bit(dirs.valuePtr()));
    } while (dirs.goNext());
    dirs.detach();

    return ret;
}

bool Alpm::cleanCacheDirs() {
    if (!isValid(true)) return false;

    QStringList dirs = Alpm::cacheDirs();
    for (int i=0;i<dirs.count();i++) {
        QDir dir(dirs.at(i));
        dir.setNameFilters(QStringList() << "*.pkg.tar.*");
        dir.setFilter(QDir::Files);
        for(QString & dirFile: dir.entryList()) {
           dir.remove(dirFile);
        }
    }

    return true;
}

bool Alpm::pkgLessThan(const AlpmPackage & pkg1,const AlpmPackage & pkg2) {
    int ret;
    if ((ret = pkg1.name().compare(pkg2.name())) != 0) return (ret < 0);
    if ((ret = AlpmPackage::pkg_vercmp(pkg1.version(),pkg2.version())) != 0) return (ret < 0);
    if (pkg1.repo() == "local") return true;
    if (pkg2.repo() == "local") return false;
    return (pkg1.repo().compare(pkg2.repo()) < 0);
}

QList<AlpmPackage> Alpm::findByPackageName(const QString & pkgname) const {
    QList<AlpmPackage> ret;
    if (!isValid(true)) return ret;

    ret += localDB().findByPackageName(pkgname);
    for (AlpmDB & db: ((Alpm *)this)->allSyncDBs()) {
        ret += db.findByPackageName(pkgname);
    }

    std::sort(ret.begin(),ret.end(),pkgLessThan);

    return ret;
}

QList<AlpmPackage> Alpm::findByPackageNameVersion(const QString & pkgname,const QString & version) const {
    QList<AlpmPackage> ret;
    if (!isValid(true)) return ret;

    ret += localDB().findByPackageNameVersion(pkgname,version);
    for (AlpmDB & db: ((Alpm *)this)->allSyncDBs()) {
        ret += db.findByPackageNameVersion(pkgname,version);
    }

    std::sort(ret.begin(),ret.end(),pkgLessThan);

    return ret;
}

QList<AlpmPackage> Alpm::findByPackageNameProvides(const AlpmPackage::Dependence & provide) const {
    QList<AlpmPackage> ret;
    if (!isValid(true)) return ret;

    ret += localDB().findByPackageNameProvides(provide);
    for (AlpmDB & db: ((Alpm *)this)->allSyncDBs()) {
        ret += db.findByPackageNameProvides(provide);
    }

    std::sort(ret.begin(),ret.end());

    return ret;
}

QList<AlpmPackage> Alpm::find(const QRegularExpression & expr) const {
    QList<AlpmPackage> ret;
    if (!isValid(true)) return ret;

    ret += localDB().find(expr);
    for (AlpmDB & db: ((Alpm *)this)->allSyncDBs()) {
        ret += db.find(expr);
    }

    std::sort(ret.begin(),ret.end(),pkgLessThan);

    return ret;
}

QStringList Alpm::groups() const {
    return m_groups;
}
