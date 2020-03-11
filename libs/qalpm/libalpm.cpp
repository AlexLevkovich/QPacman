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
#include <QDebug>

typedef QMap<QString,QString > StringStringMap;

Alpm * Alpm::p_alpm = NULL;
int Alpm::m_percent = -1;
int Alpm::prev_event_type = -1;
AlpmConfig * Alpm::p_config = NULL;

template<class ForwardIt, class T, class Compare> static ForwardIt binary_search_ex(ForwardIt first, ForwardIt last, const T& value, Compare comp) {
    ForwardIt it = std::lower_bound(first, last, value, comp);
    if ((it != last) && comp(value,*it)) it = last;
    return it;
}

Alpm::Alpm(QObject *parent) : ThreadRun(parent) {
    m_alpm_handle = NULL;
    m_alpm_errno = ALPM_ERR_OK;
    if (p_alpm == NULL) p_alpm = this;
    m_percent = -1;
    prev_event_type = -1;

    connect(&lock_watcher,SIGNAL(changed(const QString &)),this,SLOT(lockFileChanged(const QString &)));
}

Alpm::~Alpm() {
    close();
    p_alpm = NULL;
}

void Alpm::lockFileChanged(const QString & path) {
    if (path != lockFilePath() && path != m_system_lock_file) return;
    emit locking_changed(path,QFile(path).exists());
}

void Alpm::emit_information(const QString & message,bool significant) {
    if (!message.isEmpty()) QMetaObject::invokeMethod(this,"information",Qt::QueuedConnection,Q_ARG(QString,message),Q_ARG(bool,significant));
}

void Alpm::emit_event(const QString & message) {
    if (!message.isEmpty()) QMetaObject::invokeMethod(this,"event",Qt::QueuedConnection,Q_ARG(QString,message));
    emit_information(message);
}

void Alpm::emit_error(const QString & message) {
    if (!message.isEmpty()) QMetaObject::invokeMethod(this,"error",Qt::QueuedConnection,Q_ARG(QString,message));
}

void Alpm::emit_question(const QString & message,bool * answer) {
    QMetaObject::invokeMethod(this,"question",(QThread::currentThread() != qApp->thread())?Qt::BlockingQueuedConnection:Qt::DirectConnection,Q_ARG(QString,message),Q_ARG(bool *,answer));
    emit_information(message+(*answer?": Y":": N"));
}

void Alpm::emit_progress(const char * signal,const QString & pkg_name,int percent,size_t n_targets,size_t current_target) {
    QMetaObject::invokeMethod(this,signal,Qt::QueuedConnection,Q_ARG(QString,pkg_name),Q_ARG(int,percent),Q_ARG(size_t,n_targets),Q_ARG(size_t,current_target));
}

void Alpm::emit_progress(const char * signal,int percent) {
    QMetaObject::invokeMethod(this,signal,Qt::QueuedConnection,Q_ARG(int,percent));
}

void Alpm::emit_select_provider(const QString & pkgname,const QStringList & providers,int * answer) {
    QMetaObject::invokeMethod(this,"select_provider",(QThread::currentThread() != qApp->thread())?Qt::BlockingQueuedConnection:Qt::DirectConnection,Q_ARG(QString,pkgname),Q_ARG(QStringList,providers),Q_ARG(int *,answer));
    QString info = tr("You need to pick up what provider to use for %1 package:\n").arg(pkgname);
    for (int i=1;i<=providers.count();i++) {
        info += QString("%1) ").arg(i) + providers.at(i-1) + "\n";
    }
    info += tr("Select the item number: %1").arg((answer == NULL)?"":QString("%1").arg(*answer));
    emit_information(info);
}

void Alpm::emit_optdepends_event(const QString & pkgname,const StringStringMap & installed_deps,const StringStringMap & pending_deps) {
    QMetaObject::invokeMethod(this,"optdepends_event",Qt::QueuedConnection,Q_ARG(QString,pkgname),Q_ARG(StringStringMap,installed_deps),Q_ARG(StringStringMap,pending_deps));
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
    QMetaObject::invokeMethod(this,"install_packages_confirmation",(QThread::currentThread() != qApp->thread())?Qt::BlockingQueuedConnection:Qt::DirectConnection,Q_ARG(QStringList,install),Q_ARG(QStringList,remove),Q_ARG(qint64,dl_size),Q_ARG(qint64,install_size),Q_ARG(qint64,remove_size),Q_ARG(bool *,ok));
    QMetaObject::invokeMethod(this,"full_download_size_found",(QThread::currentThread() != qApp->thread())?Qt::BlockingQueuedConnection:Qt::DirectConnection,Q_ARG(qint64,dl_size));
    QString info;
    if (remove.count() > 0) {
        info += tr("The following packages will be removed:\n");
        info += remove.join("\n");
        info += "\n";
    }
    info += tr("The following packages will be installed:\n");
    info += install.join("\n");
    info += "\n";
    info += tr("Do you agree? : ") + (*ok?"Y":"N");
    p_alpm->emit_information(info);
}

void Alpm::emit_remove_packages_confirmation(const QStringList & _remove,qint64 remove_size,bool * ok) {
    QStringList remove = _remove;
    std::sort(remove.begin(),remove.end(),string_name_cmp);
    QMetaObject::invokeMethod(this,"remove_packages_confirmation",(QThread::currentThread() != qApp->thread())?Qt::BlockingQueuedConnection:Qt::DirectConnection,Q_ARG(QStringList,remove),Q_ARG(qint64,remove_size),Q_ARG(bool *,ok));
    QString info = tr("The following packages will be removed:\n");
    info += remove.join("\n");
    info += "\n";
    info += tr("Do you agree? : ") + (*ok?"Y":"N");
    p_alpm->emit_information(info);
}

bool Alpm::open(const QString & confpath,const QString & dbpath) {
    if (isValid() || p_alpm != this) {
        m_alpm_errno = ALPM_INSTANCE_ALREADY_CREATED;
        return false;
    }

    if (!config.setConfPath(confpath)) {
        m_alpm_errno = CANNOT_LOAD_CONFIG;
        return false;
    }
    p_config = &config;

    if (!dbpath.isEmpty() && ::getuid() != 0) {
        m_alpm_handle = config.translate();
        if (!isValid()) {
            m_alpm_errno = ALPM_HANDLE_FAILED;
            return false;
        }

        QString new_link = QString("%1/local").arg(dbpath);
        QFileInfo link_fi(new_link);
        if (!link_fi.isSymLink() && link_fi.exists()) {
            m_alpm_errno = ALPM_LINK_LOCAL_DB_FAILED;
            return false;
        }
        if (!link_fi.isSymLink() || !link_fi.exists()) {
            if (symlink(QString("%1/local").arg(QString::fromLocal8Bit(alpm_option_get_dbpath(m_alpm_handle))).toLocal8Bit().constData(),new_link.toLocal8Bit().constData()) == -1) {
                m_alpm_errno = ALPM_LINK_LOCAL_DB_FAILED;
                return false;
            }
        }
        m_system_lock_file = lockFilePath();
        alpm_release(m_alpm_handle);
    }
    m_alpm_handle = config.translate(dbpath);
    if (!isValid()) {
        m_alpm_errno = ALPM_HANDLE_FAILED;
        return false;
    }

    alpm_option_set_progresscb(m_alpm_handle,operation_progress_fn);
    alpm_option_set_questioncb(m_alpm_handle,operation_question_fn);
    alpm_option_set_eventcb(m_alpm_handle,operation_event_fn);
    alpm_option_set_fetchcb(m_alpm_handle,operation_fetch_fn);

    m_confpath = confpath;
    m_dbpath = dbpath;
    lock_watcher.addPath(QFileInfo(lockFilePath()).dir().path());
    if (!m_system_lock_file.isEmpty()) lock_watcher.addPath(QFileInfo(m_system_lock_file).dir().path());

    recreatedbs();

    return true;
}

void Alpm::recreatedbs() {
    m_localDB = AlpmDB(alpm_get_localdb(m_alpm_handle));
    m_syncDBs.clear();

    AlpmList<alpm_db_t> dbs(alpm_get_syncdbs(m_alpm_handle),AlpmList<alpm_db_t>::ignorefree);
    do {
        if (dbs.isEmpty()) break;
        m_syncDBs.append(AlpmDB(dbs.valuePtr()));
    } while (dbs.next());
    dbs.detach();
}

bool Alpm::isOpen() {
    return (p_alpm != NULL && p_alpm->m_alpm_handle != NULL);
}

bool Alpm::reopen() {
    if (!isValid(true)) return false;

    QString dbpath = m_dbpath;
    QString confpath = m_confpath;
    close();
    return open(confpath,dbpath);
}

void Alpm::close() {
    if (!isValid()) return;

    AlpmDB::clean_pkg_caches();
    lock_watcher.removeAllPaths();
    alpm_release(m_alpm_handle);
    m_alpm_handle = NULL;
    m_system_lock_file.clear();
    m_confpath.clear();
    m_dbpath.clear();
    m_groups.clear();
}

Alpm * Alpm::instance() {
    return p_alpm;
}

int Alpm::operation_fetch_fn(const QString & url,const QString & localpath,bool) {
    AlpmDownloader * downloader = new AlpmDownloader(QUrl(url),localpath,AlpmConfig::downloaderThreadCount(),p_config->doDisableDownloadTimeout()?0:AlpmConfig::downloaderTimeout(),AlpmConfig::downloaderProxy());
    QObject::connect(downloader,SIGNAL(progress(const QString &,qint64,qint64,int,qint64)),p_alpm,SLOT(operation_download_fn(const QString &,qint64,qint64,int,qint64)));
    QObject::connect(downloader,SIGNAL(error(const QString &)),p_alpm,SIGNAL(information(const QString &)));
    QObject::connect(downloader,SIGNAL(error(const QString &)),p_alpm,SIGNAL(error(const QString &)));
    int ret = downloader->exec();
    delete downloader;
    return (ret == 2)?-1:ret;
}

void Alpm::operation_download_fn(const QString & filename,qint64 bytes_downloaded,qint64 length,int,qint64) {
    if(bytes_downloaded == 0 && length <= 0) return;
    if (length < 0) length = 0;
    emit_information(tr("(%1/%2) downloaded").arg(bytes_downloaded).arg(length));
    QMetaObject::invokeMethod(this,"download_progress",Qt::QueuedConnection,Q_ARG(QString,filename),Q_ARG(qint64,bytes_downloaded),Q_ARG(qint64,length));
}

int Alpm::operation_fetch_fn(const char *url, const char *localpath,int force) {
    return (int)Alpm::operation_fetch_fn(QString::fromLocal8Bit(url),QString::fromLocal8Bit(localpath),(int)force);
}

void Alpm::operation_progress_fn(alpm_progress_t progress, const char * pkg_name, int percent, size_t n_targets, size_t current_target) {
    if (m_percent != -1 && m_percent == percent) return;

    m_percent = percent;
    QString pkgname = QString::fromLocal8Bit((pkg_name == NULL)?"":pkg_name);
    switch (progress) {
    case ALPM_PROGRESS_ADD_START:
        p_alpm->emit_information(tr("(%1/%2) Installing %3 (%4%)").arg(current_target).arg(n_targets).arg(pkgname).arg(percent));
        p_alpm->emit_progress("install_progress",pkgname,percent,n_targets,current_target);
        break;
    case ALPM_PROGRESS_REINSTALL_START:
        p_alpm->emit_information(tr("(%1/%2) Reinstalling %3 (%4%)").arg(current_target).arg(n_targets).arg(pkgname).arg(percent));
        p_alpm->emit_progress("install_progress",pkgname,percent,n_targets,current_target);
        break;
    case ALPM_PROGRESS_UPGRADE_START:
        p_alpm->emit_information(tr("(%1/%2) Upgrading %3 (%4%)").arg(current_target).arg(n_targets).arg(pkgname).arg(percent));
        p_alpm->emit_progress("install_progress",pkgname,percent,n_targets,current_target);
        break;
    case ALPM_PROGRESS_DOWNGRADE_START:
        p_alpm->emit_information(tr("(%1/%2) Downgrading %3 (%4%)").arg(current_target).arg(n_targets).arg(pkgname).arg(percent));
        p_alpm->emit_progress("install_progress",pkgname,percent,n_targets,current_target);
        break;
    case ALPM_PROGRESS_REMOVE_START:
        p_alpm->emit_information(tr("(%1/%2) Removing %3 (%4%)").arg(current_target).arg(n_targets).arg(pkgname).arg(percent));
        p_alpm->emit_progress("remove_progress",pkgname,percent,n_targets,current_target);
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

void Alpm::operation_question_fn(alpm_question_t * question) {
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
                QString message(QObject::tr("The following package(s) cannot be upgraded due to unresolvable dependencies:\n"));
                message += namelist.toString(alpm_item_string_fn,QString("     "));
                message += QObject::tr("Do you want to skip the above package(s) for this upgrade?");
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
                } while (namelist.next());
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
    } while(depends.next());
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
    } while(depends.next());

    p_alpm->emit_information(info);
    p_alpm->emit_optdepends_event(pkgname,installed_deps,pending_deps);
}

class OverwriteHandler {
private:
    OverwriteHandler(const QList<AlpmPackage *> & forcedpkgs) {
        m_forcedpkgs = forcedpkgs;
    }

    ~OverwriteHandler() {
        if (Alpm::instance() == NULL || Alpm::instance()->m_alpm_handle == NULL) return;

        AlpmList<char> overwrite_files(alpm_list_copy(alpm_option_get_overwrite_files(Alpm::instance()->m_alpm_handle)),AlpmList<char>::ignorefree);
        do {
            if (overwrite_files.isEmpty()) break;
            alpm_option_remove_overwrite_file(Alpm::instance()->m_alpm_handle,(const char *)overwrite_files.valuePtr());
        } while(overwrite_files.next());
        m_forcedpkgs.clear();
    }

    static void addPkgFiles() {
        if (Alpm::instance() == NULL || Alpm::instance()->m_alpm_handle == NULL) return;

        QString pkg_cache_path;
        for (int i=0;i<m_forcedpkgs.count();i++) {
            pkg_cache_path.clear();
            if (!m_forcedpkgs[i]->isDownloaded(&pkg_cache_path) || pkg_cache_path.isEmpty()) continue;
            addFiles(AlpmPackage(pkg_cache_path).files());
        }
    }

    static void addFiles(const QStringList & files) {
        for (int i=0;i<files.count();i++) {
            QFileInfo info(files[i]);
            if (!info.exists() || (!info.isFile() && !info.isSymLink()) || (info.isDir() && !info.isSymLink())) continue;
            alpm_option_add_overwrite_file(Alpm::instance()->m_alpm_handle,files.at(i).toLocal8Bit().constData());
        }
    }

    static QList<AlpmPackage *> m_forcedpkgs;
    friend class Alpm;
};

QList<AlpmPackage *> OverwriteHandler::m_forcedpkgs;

void Alpm::operation_event_fn(alpm_event_t * event) {
    m_percent = -1;
    switch(event->type) {
        case ALPM_EVENT_HOOK_START:
            if(event->hook.when == ALPM_HOOK_PRE_TRANSACTION) {
                p_alpm->emit_event(QObject::tr("Running pre-transaction hooks..."));
            } else {
                p_alpm->emit_event(QObject::tr("Running post-transaction hooks..."));
            }
            break;
        case ALPM_EVENT_HOOK_RUN_START:
            {
                alpm_event_hook_run_t *e = &event->hook_run;
                QString txt = e->desc?QString::fromLocal8Bit(e->desc):QString::fromLocal8Bit(e->name);
                QMetaObject::invokeMethod(p_alpm,"hook",Qt::QueuedConnection,Q_ARG(QString,txt),Q_ARG(int,(int)e->position),Q_ARG(int,(int)e->total));
                p_alpm->emit_information(QString("%1/%2 %3").arg(e->position).arg(e->total).arg(txt));
            }
            break;
        case ALPM_EVENT_CHECKDEPS_START:
            p_alpm->emit_event(QObject::tr("Checking dependencies..."));
            break;
        case ALPM_EVENT_FILECONFLICTS_START:
            p_alpm->emit_event(QObject::tr("Checking for file conflicts..."));
            OverwriteHandler::addPkgFiles();
            break;
        case ALPM_EVENT_RESOLVEDEPS_START:
            p_alpm->emit_event(QObject::tr("Resolving dependencies..."));
            break;
        case ALPM_EVENT_INTERCONFLICTS_START:
            p_alpm->emit_event(QObject::tr("Looking for conflicting packages..."));
            break;
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
                        break;
                    case ALPM_PACKAGE_UPGRADE:
                    case ALPM_PACKAGE_DOWNGRADE:
                        display_new_optdepends(e->oldpkg, e->newpkg);
                        break;
                    case ALPM_PACKAGE_REINSTALL:
                    case ALPM_PACKAGE_REMOVE:
                        break;
                }
            }
            break;
        case ALPM_EVENT_INTEGRITY_START:
            p_alpm->emit_event(QObject::tr("Checking package integrity..."));
            break;
        case ALPM_EVENT_KEYRING_START:
            p_alpm->emit_event(QObject::tr("Checking keyring..."));
            break;
        case ALPM_EVENT_KEY_DOWNLOAD_START:
            p_alpm->emit_information(QObject::tr("Downloading required keys..."));
            break;
        case ALPM_EVENT_LOAD_START:
            p_alpm->emit_event(QObject::tr("Loading package files..."));
            break;
        case ALPM_EVENT_SCRIPTLET_INFO:
        {
            if (prev_event_type != ALPM_EVENT_SCRIPTLET_INFO) p_alpm->emit_event(QObject::tr("Executing an internal scriplet..."));
            p_alpm->emit_information(QString::fromLocal8Bit((const char *)event->scriptlet_info.line));
            break;
        }
        case ALPM_EVENT_RETRIEVE_START:
            p_alpm->emit_information(QObject::tr("Retrieving packages..."));
            QMetaObject::invokeMethod(p_alpm,"download_starting",Qt::QueuedConnection);
            break;
        case ALPM_EVENT_DISKSPACE_START:
            p_alpm->emit_event(QObject::tr("Checking available disk space..."));
            break;
        case ALPM_EVENT_OPTDEP_REMOVAL:
            {
                alpm_event_optdep_removal_t *e = &event->optdep_removal;
                char * dep_string = alpm_dep_compute_string(e->optdep);
                p_alpm->emit_information(QObject::tr("%1 optionally requires %2").arg(QString::fromLocal8Bit((const char *)alpm_pkg_get_name(e->pkg))).
                                                                                  arg(QString::fromLocal8Bit((const char *)dep_string)));
                free(dep_string);
            }
            break;
        case ALPM_EVENT_DATABASE_MISSING:
            p_alpm->emit_information(QObject::tr("Database file for '%1' does not exist, you need to update your databases!").
                                                                                  arg(QString::fromLocal8Bit((const char *)event->database_missing.dbname)));
            break;
        case ALPM_EVENT_PACNEW_CREATED:
            {
                alpm_event_pacnew_created_t *e = &event->pacnew_created;
                p_alpm->emit_information(QObject::tr("%1 installed as %12.pacnew").arg(QString::fromLocal8Bit((const char *)e->file)));
            }
            break;
        case ALPM_EVENT_PACSAVE_CREATED:
            {
                alpm_event_pacsave_created_t *e = &event->pacsave_created;
                p_alpm->emit_information(QObject::tr("%1 saved as %1.pacsave").arg(QString::fromLocal8Bit((const char *)e->file)));
            }
            break;
        /* all the simple done events, with fallthrough for each */
        case ALPM_EVENT_FILECONFLICTS_DONE:
        case ALPM_EVENT_CHECKDEPS_DONE:
        case ALPM_EVENT_RESOLVEDEPS_DONE:
        case ALPM_EVENT_INTERCONFLICTS_DONE:
        case ALPM_EVENT_TRANSACTION_DONE:
        case ALPM_EVENT_INTEGRITY_DONE:
        case ALPM_EVENT_KEYRING_DONE:
        case ALPM_EVENT_KEY_DOWNLOAD_DONE:
        case ALPM_EVENT_LOAD_DONE:
        case ALPM_EVENT_DISKSPACE_DONE:
            break;
        case ALPM_EVENT_RETRIEVE_DONE:
            p_alpm->emit_information(QObject::tr("Packages have been retrieved successfully!"));
            QMetaObject::invokeMethod(p_alpm,"download_completed",Qt::QueuedConnection);
            break;
        case ALPM_EVENT_RETRIEVE_FAILED:
            p_alpm->emit_information(QObject::tr("Packages have not been retrieved :("));
            QMetaObject::invokeMethod(p_alpm,"download_failed",Qt::QueuedConnection);
            break;
        case ALPM_EVENT_HOOK_DONE:
        case ALPM_EVENT_HOOK_RUN_DONE:
            break;
        case ALPM_EVENT_PKGDOWNLOAD_START:
            {
                alpm_event_pkgdownload_t *e = &event->pkgdownload;
                QString filename = QString::fromLocal8Bit((const char *)e->file);
                p_alpm->emit_information(QObject::tr("Starting the download of %1").arg(filename));
                QMetaObject::invokeMethod(p_alpm,"download_start",Qt::QueuedConnection,Q_ARG(QString,filename));
            }
            break;
        case ALPM_EVENT_PKGDOWNLOAD_DONE:
            {
                alpm_event_pkgdownload_t *e = &event->pkgdownload;
                QString filename = QString::fromLocal8Bit((const char *)e->file);
                p_alpm->emit_information(QObject::tr("Done the download of %1").arg(filename));
                QMetaObject::invokeMethod(p_alpm,"download_done",Qt::QueuedConnection,Q_ARG(QString,filename));
            }
            break;
        case ALPM_EVENT_PKGDOWNLOAD_FAILED:
            {
                alpm_event_pkgdownload_t *e = &event->pkgdownload;
                QString filename = QString::fromLocal8Bit((const char *)e->file);
                p_alpm->emit_information(QObject::tr("Failed the download of %1").arg(filename));
                QMetaObject::invokeMethod(p_alpm,"download_failed",Qt::QueuedConnection,Q_ARG(QString,filename));
            }
            break;
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

Alpm::CapsFlags Alpm::capabilities() {
    return (Alpm::CapsFlags)alpm_capabilities();
}

QString Alpm::arch() const {
    if (!isValid(true)) return QString();

    const char * arch = alpm_option_get_arch(m_alpm_handle);
    if (arch == NULL) return QString();
    return QString::fromLocal8Bit(arch);
}

QString Alpm::lastError() const {
    if (m_alpm_errno != ALPM_ERR_OK) {
        if (m_alpm_errno < 0) {
            switch (m_alpm_errno) {
            case PKG_LIST_IS_EMPTY:
                return tr("The input package list is empty!");
            case ALPM_INSTANCE_ALREADY_CREATED:
                return tr("Alpm class has been already initialized!");
            case ALPM_IS_NOT_OPEN:
                return tr("Alpm class is open yet!");
            case ALPMDB_HANDLE_FAILED:
                return tr("Alpm's DB handle is not initialized!");
            case LOCAL_DB_UPDATE:
                return tr("Local DB does not need in updating!");
            case PKG_IS_NOT_INITED:
                return tr("The package class has not been properly intialized!");
            case REASON_WRONG_DB:
                return tr("The reason can be changed only for local packages!");
            case LOCK_FILE_EXISTS:
                return tr("The lock file exists!");
            case CANNOT_GET_ROOT:
                return tr("Cannot obtain the root rights!");
            case ALPM_CONFIG_FAILED:
                return tr("Cannot load config file!");
            case ALPM_LINK_LOCAL_DB_FAILED:
                return tr("Cannot create the link for local's db!");
            case ALPM_HANDLE_FAILED:
                return tr("Cannot create Alpm handle from config handle!");
            case THREAD_IS_ALREADY_RUNNING:
                return tr("Cannot invoke the function in the thread if other one is still working!");
            case CANNOT_LOAD_CONFIG:
                return config.lastError();
            }
        }
        else return QString::fromLocal8Bit(alpm_strerror((alpm_errno_t)m_alpm_errno));
    }

    if (!isValid()) return QString::fromLocal8Bit(alpm_strerror(ALPM_ERR_HANDLE_NULL));

    alpm_errno_t err = alpm_errno(m_alpm_handle);
    if (err == ALPM_ERR_OK) return QString();
    return QString::fromLocal8Bit(alpm_strerror(err));
}

QList<AlpmDB> Alpm::allSyncDBs() const {
    return m_syncDBs;
}

AlpmDB Alpm::localDB() const {
    return m_localDB;
}

bool Alpm::dup_repo_cmp(AlpmPackage * pkg1, AlpmPackage * pkg2) {
    int ret = pkg1->name().compare(pkg2->name());
    if (ret) return false;
    ret = pkg1->version().compare(pkg2->version());
    if (ret) return false;
    if (pkg1->repo() == "local" || pkg2->repo() == "local") return true;
    return pkg1->repo().compare(pkg2->repo()) == 0;
}

bool Alpm::sort_cmp(AlpmPackage * item1,AlpmPackage * item2) {
    int ret;
    if ((ret = item1->name().compare(item2->name())) != 0) return (ret < 0);
    return (Alpm::pkg_vercmp(item1->version(),item2->version()) < 0);
}

bool Alpm::sort_equal_cmp(AlpmPackage * item1,AlpmPackage * item2) {
    if (item1->name().compare(item2->name())) return false;
    return !Alpm::pkg_vercmp(item1->version(),item2->version());
}

const QVector<AlpmPackage *> & Alpm::lastQueriedPackages() const {
    return m_packages;
}

void Alpm::fill_replaces() {
    m_replaces.clear();
    AlpmDB local_db = localDB();
    QVector<AlpmPackage *> res;
    QList<AlpmPackage::Dependence> replaces;
    AlpmPackage::Dependence dep;
    for (int i=0;i<m_packages.count();i++) {
        if (m_packages[i]->isIgnorable()) continue;
        replaces = m_packages[i]->replaces();
        if (replaces.isEmpty()) continue;
        for (int j=0;j<replaces.count();j++) {
            dep = replaces[j];
            res = local_db.findByPackageName(dep.name());
            if (res.isEmpty()) continue;
            if (dep.isAppropriate(res[0])) m_replaces[res[0]] << m_packages[i];
        }
    }
}

void Alpm::query_packages_portion(QVector<AlpmPackage *> & pkgs,int startindex,int lastindex) {
    QVector<int> repo_local_equals;
    int i;
    int local_index = (pkgs[startindex]->repo() == "local")?startindex:-1;
    for (i=startindex;i<=lastindex;i++) {
        if (i == local_index) continue;
        if (local_index >= 0 && sort_equal_cmp(pkgs[i],pkgs[local_index])) repo_local_equals.append(i);
    }
    if (repo_local_equals.count() > 1) {
        for (i=0;i<repo_local_equals.count();i++) {
            pkgs[repo_local_equals[i]]->setInstalled(false);
        }
    }
    else if (repo_local_equals.count() == 1) pkgs[local_index] = NULL;
}

int Alpm::query_packages() {
    m_groups.clear();
    m_packages.clear();
    AlpmDB::clean_pkg_caches();

    int i;
    QList<AlpmDB> dbs = allSyncDBs();
    for (i=0;i<dbs.count();i++) {
        m_packages += dbs[i].packages();
        m_groups += dbs[i].groups();
    }

    fill_replaces();

    m_packages += localDB().packages();
    m_groups += localDB().groups();
    m_groups.removeDuplicates();

    if (m_packages.count() <= 0) {
        return alpm_errno(Alpm::instance()->m_alpm_handle);
    }

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

    m_packages.erase(std::remove_if(m_packages.begin(),m_packages.end(),[](AlpmPackage * pkg){return !pkg;}),m_packages.end());

    return ALPM_ERR_OK;
}

const QStringList Alpm::dirContents(const QDir & dir,const QString & nameFilter) {
    QStringList ret;
    QDirIterator it(dir.path(),QStringList() << nameFilter);
    while (it.hasNext()) {
        ret << it.next();
    }
    return ret;
}

bool Alpm::queryPackages(bool use_eventloop) {
    if (!isValid(true)) return false;

    for (QString filename: dirContents(dbDirPath()+QDir::separator()+QString::fromLatin1("sync"),"*.part")) {
        QFile(filename).remove();
    }

    if (use_eventloop) {
        bool ret = (run<int>(m_alpm_errno,this,&Alpm::query_packages) == ThreadRun::OK) && m_alpm_errno == ALPM_ERR_OK;
        QMetaObject::invokeMethod(this,"listing_packages_completed",Qt::QueuedConnection);
        return ret;
    }

    return (query_packages() == ALPM_ERR_OK);
}

ThreadRun::RC Alpm::updateDBs(bool force) {
    if (!isValid(true)) return ThreadRun::BAD;

    ThreadRun::RC rc = run<int>(m_alpm_errno,this,&Alpm::update_dbs,force);
    emit dbs_updated(rc == ThreadRun::OK && m_alpm_errno == ALPM_ERR_OK);
    return (m_alpm_errno != ALPM_ERR_OK && rc == ThreadRun::OK)?ThreadRun::BAD:rc;
}

int Alpm::update_dbs(bool force) {
    m_percent = -1;

    QList<AlpmDB> dbs = allSyncDBs();
    for (int i=0;i<dbs.count();i++) {
        QMetaObject::invokeMethod(this,"download_db_start",Qt::QueuedConnection,Q_ARG(QString,dbs[i].name()));
        emit_information(QObject::tr("Updating %1 db...").arg(dbs[i].name()));
        if (!dbs[i].update(force)) {
            emit_information(lastError());
            emit_error(lastError());
            return alpm_errno(Alpm::instance()->m_alpm_handle);
        }
    }

    return ALPM_ERR_OK;
}

QStringList Alpm::download_packages(qint64 download_size,const QStringList & download_urls) {
    QStringList downloaded_paths;
    if (!isValid(true)) return downloaded_paths;

    m_percent = -1;

    if (download_urls.count() <= 0 || download_size <= 0) {
        QMetaObject::invokeMethod(p_alpm,"error",Qt::QueuedConnection,Q_ARG(QString,tr("Nothing to download!!!")));
        QMetaObject::invokeMethod(p_alpm,"download_failed",Qt::QueuedConnection);
        return downloaded_paths;
    }

    QMetaObject::invokeMethod(this,"full_download_size_found",Qt::QueuedConnection,Q_ARG(qint64,download_size));

    QString filename;
    for (int i=0;i<download_urls.count();i++) {
        filename = QUrl(download_urls.at(i)).fileName();
        emit_information(tr("Starting the download of %1").arg(filename));
        QMetaObject::invokeMethod(p_alpm,"download_start",Qt::QueuedConnection,Q_ARG(QString,filename));
        if (Alpm::operation_fetch_fn(download_urls.at(i),cacheDirs().at(0),true) == -1) {
            QMetaObject::invokeMethod(p_alpm,"download_failed",Qt::QueuedConnection);
            return downloaded_paths;
        }
        downloaded_paths.append(cacheDirs().at(0)+QDir::separator()+filename);
    }
    QMetaObject::invokeMethod(p_alpm,"download_completed",Qt::QueuedConnection);

    return downloaded_paths;
}

ThreadRun::RC Alpm::downloadPackages(const QList<AlpmPackage *> & pkgs,QStringList * paths) {
    if (!isValid(true)) return ThreadRun::BAD;

    QStringList download_urls;
    qint64 download_size = 0;
    alpm_pkg_t * handle;
    QString loc;
    int i;
    for (i=0;i<pkgs.count();i++) {
        loc = pkgs.at(i)->remoteLocation();
        if (loc.isEmpty()) continue;
        handle = pkgs.at(i)->handle();
        if (handle == NULL) continue;
        download_size += alpm_pkg_download_size(handle);
        download_urls.append(loc+QDir::separator()+pkgs.at(i)->fileName());
    }

    QStringList downloaded_paths;
    ThreadRun::RC rc = run<QStringList>(downloaded_paths,this,&Alpm::download_packages,download_size,download_urls);
    emit pkgs_downloaded(downloaded_paths);
    if (rc == ThreadRun::OK && paths != NULL) *paths = downloaded_paths;
    return (downloaded_paths.count() <= 0 && rc == ThreadRun::OK)?ThreadRun::BAD:rc;
}

bool Alpm::updates_cmp(AlpmPackage * item1,AlpmPackage * item2) {
    return (Alpm::pkg_vercmp(item1->version(),item2->version()) > 0 &&
            item1->name().compare(item2->name()) == 0);
}

int Alpm::find_updates_in_list(const QVector<AlpmPackage *> & m_list,AlpmPackage * value) {
    if (m_list.count() <= 0) return -1;

    for (int i=0;i<m_list.count();i++) {
        if(updates_cmp((AlpmPackage *)m_list[i],value)) return i;
    }

    return -1;
}

QVector<AlpmPackage *> Alpm::check_updates() {
    QVector<AlpmPackage *> ret;

    queryPackages(false);
    const QVector<AlpmPackage *> & pkgs = lastQueriedPackages();

    for (int i=0;i<pkgs.count();i++) {
        if (pkgs[i]->isUpdate()) ret.append(pkgs[i]);
    }

    std::sort(ret.begin(),ret.end(),sort_cmp);
    ret.erase(std::unique(ret.begin(),ret.end(),sort_equal_cmp),ret.end());

    return ret;
}

QVector<AlpmPackage *> Alpm::updates() const {
    QVector<AlpmPackage *> ret;
    if (!isValid(true)) return ret;

    Alpm * p_this = (Alpm *)this;
    p_this->run<QVector<AlpmPackage *> >(ret,p_this,&Alpm::check_updates);
    return ret;
}

bool Alpm::do_process_targets(bool remove,QStringList & install_targets,QStringList & remove_targets) {
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
        } while(list.next());
    }
    list.detach();

    if (rlist.count() > 0) {
        do {
            alpm_pkg_t *pkg = rlist.valuePtr();
            remove_size += alpm_pkg_get_isize(pkg);
            remove_targets.append(QString("%3/%1=%2").arg(QString::fromLocal8Bit(alpm_pkg_get_name(pkg))).arg(QString::fromLocal8Bit(alpm_pkg_get_version(pkg))).arg(QString::fromLocal8Bit((db_name = (char *)alpm_db_get_name(alpm_pkg_get_db(pkg)))?db_name:"local")));
        } while(rlist.next());
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
        if (install_targets.count() > 0) QMetaObject::invokeMethod(Alpm::instance(),"pkgs_installed",Qt::QueuedConnection,Q_ARG(QStringList,install_targets),Q_ARG(QStringList,remove_targets));
        else if (remove_targets.count() > 0) QMetaObject::invokeMethod(Alpm::instance(),"pkgs_removed",Qt::QueuedConnection,Q_ARG(QStringList,remove_targets));
    }
    void setPkgs(const QStringList & install_targets,const QStringList & remove_targets) {
        this->install_targets = install_targets;
        this->remove_targets = remove_targets;
    }
private:
    alpm_handle_t * m_handle;
    QStringList install_targets;
    QStringList remove_targets;
};

bool Alpm::only_pkg_name_cmp(AlpmPackage * item1,AlpmPackage * item2) {
    return (item1->name().compare(item2->name()) < 0);
}

void Alpm::sync_sysupgrade_portion(QVector<AlpmPackage *> & add_pkgs,int startindex,int lastindex) {
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
    if (_startindex == _lastindex) return;

    for (i=(_startindex-1);i>=startindex;i--) {
        add_pkgs[i] = NULL;
    }

    int sel_index = _startindex;
    alpm_question_select_provider_t provider_question;
    provider_question.type = ALPM_QUESTION_SELECT_PROVIDER;
    provider_question.use_index = 0;
    alpm_list_t *providers = NULL;
    for (i=_startindex;i<=_lastindex;i++) {
        providers = alpm_list_add(providers,add_pkgs[i]->handle());
    }
    provider_question.providers = providers;
    alpm_depend_t alpm_dep = AlpmPackage::Dependence(add_pkgs[_startindex]).to_alpm_depend();
    provider_question.depend = &alpm_dep;
    operation_question_fn((alpm_question_t *)&provider_question);
    sel_index = _startindex + provider_question.use_index;
    free(alpm_dep.name);
    free(alpm_dep.desc);
    free(alpm_dep.version);
    alpm_list_free(providers);

    for (i=_lastindex;i>=_startindex;i--) {
        if (i == sel_index) continue;
        add_pkgs[i] = NULL;
    }
}

bool Alpm::sync_sysupgrade() {
    if (m_packages.isEmpty()) queryPackages(false);

    QVector<AlpmPackage *> add_pkgs;
    QVector<AlpmPackage *> remove_pkgs;
    int sel_index = 0;
    alpm_question_replace_t question;
    question.type = ALPM_QUESTION_REPLACE_PKG;
    alpm_question_select_provider_t provider_question;
    provider_question.type = ALPM_QUESTION_SELECT_PROVIDER;
    QVector<AlpmPackage *> new_pkgs;
    int j;
    QMapIterator<AlpmPackage *,QVector<AlpmPackage *> > i(m_replaces);
    while (i.hasNext()) {
        i.next();
        new_pkgs = i.value();

        sel_index = 0;
        provider_question.use_index = 0;
        if (new_pkgs.count() > 1) {
            alpm_list_t *providers = NULL;
            for (j=0;j<new_pkgs.count();j++) {
                providers = alpm_list_add(providers,new_pkgs[j]->handle());
            }
            provider_question.providers = providers;
            AlpmPackage::Dependence dep(i.key());
            alpm_depend_t alpm_dep = dep.to_alpm_depend();
            provider_question.depend = &alpm_dep;
            operation_question_fn((alpm_question_t *)&provider_question);
            sel_index = provider_question.use_index;
            free(alpm_dep.name);
            free(alpm_dep.desc);
            free(alpm_dep.version);
            alpm_list_free(providers);
        }

        question.replace = 0;
        question.oldpkg = i.key()->handle(),
        question.newpkg = i.value()[sel_index]->handle(),
        question.newdb = alpm_pkg_get_db(question.newpkg);
        operation_question_fn((alpm_question_t *)&question);
        if (question.replace) {
            add_pkgs.append(i.value()[sel_index]);
            remove_pkgs.append(i.key());
        }
    }

    for (j=0;j<m_packages.count();j++) {
        if (!m_packages[j]->isUpdate()) continue;

        add_pkgs.append(m_packages[j]);
    }
    std::sort(remove_pkgs.begin(),remove_pkgs.end(),sort_cmp);
    std::sort(add_pkgs.begin(),add_pkgs.end(),sort_cmp);

    QVector<AlpmPackage *>::const_iterator it;
    for (j=0;j<remove_pkgs.count();j++) {
        it = binary_search_ex(add_pkgs.begin(),add_pkgs.end(),remove_pkgs[j],only_pkg_name_cmp);
        if (it == add_pkgs.end()) continue;
        add_pkgs[it-add_pkgs.begin()] = NULL;
        j--;
    }

    add_pkgs.erase(std::remove_if(add_pkgs.begin(),add_pkgs.end(),[](AlpmPackage * pkg){return !pkg;}),add_pkgs.end());

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

    add_pkgs.erase(std::remove_if(add_pkgs.begin(),add_pkgs.end(),[](AlpmPackage * pkg){return !pkg;}),add_pkgs.end());

    for (j=0;j<remove_pkgs.count();j++) {
        if (alpm_remove_pkg(m_alpm_handle,remove_pkgs[j]->handle())) {
            QString err = QString("%1: %2").arg(remove_pkgs[j]->name()).arg(lastError());
            emit_information(err);
            emit_error(err);
            return false;
        }
    }

    for (j=0;j<add_pkgs.count();j++) {
        if (alpm_add_pkg(m_alpm_handle,add_pkgs[j]->handle()) && alpm_errno(m_alpm_handle) != ALPM_ERR_TRANS_DUP_TARGET) {
            QString err = QString("%1: %2").arg(add_pkgs[j]->name()).arg(lastError());
            emit_information(err);
            emit_error(err);
            return false;
        }
    }

    return true;
}

int Alpm::install_packages(const QList<AlpmPackage *> & m_pkgs,int m_install_flags,const QList<AlpmPackage *> & forcedpkgs) {
    m_percent = -1;
    prev_event_type = -1;
    QString err;

    HandleReleaser handle_releaser(m_alpm_handle);
    OverwriteHandler handler(forcedpkgs);

    if (alpm_trans_init(m_alpm_handle,m_install_flags)) {
        emit_information(lastError());
        emit_error(lastError());
        return alpm_errno(m_alpm_handle);
    }

    alpm_pkg_t * handle;
    for (int i=0;i<m_pkgs.count();i++) {
        handle = m_pkgs[i]->handle();
        if (handle == NULL) {
            emit_information(tr("Skipping target: %1").arg(m_pkgs[i]->toString()));
            continue;
        }
        int ret = alpm_add_pkg(m_alpm_handle,handle);
        if (ret) {
            if(alpm_errno(m_alpm_handle) == ALPM_ERR_TRANS_DUP_TARGET) {
                emit_information(tr("Skipping target: %1").arg(m_pkgs[i]->toString()));
                continue;
            }
            err = tr("%1: %2").arg(m_pkgs[i]->name()).arg(lastError());
            emit_information(err);
            emit_error(err);
            return alpm_errno(m_alpm_handle);
        }
    }

    if (m_pkgs.count() <= 0) {
        emit_information(tr("Starting full system upgrade..."),true);
        alpm_logaction(m_alpm_handle,LOGPREFIX,"starting full system upgrade\n");
        if(!sync_sysupgrade()) return alpm_errno(m_alpm_handle);
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
            } while(list.next());
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
            } while(list.next());
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
            } while(list.next());
            data = NULL;
            break;
        }
        default:
            emit_information(lastError());
            emit_error(lastError());
            FREELIST(data);
            break;
        }
        return alpm_errno(m_alpm_handle);
    }

    FREELIST(data);
    if (alpm_list_count(alpm_trans_get_add(m_alpm_handle)) <= 0) {
        emit_information(tr("No packages were upgraded because there is nothing to install."),true);
        return ALPM_ERR_OK;
    }

    QStringList install_targets;
    QStringList remove_targets;
    if (!do_process_targets(false,install_targets,remove_targets)) {
        emit_information(tr("No packages were upgraded because of user's refusal."),true);
        return USER_REFUSAL;
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
            } while(list.next());
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
            } while(list.next());
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
        return alpm_errno(m_alpm_handle);
    }

    handle_releaser.setPkgs(install_targets,remove_targets);

    emit_information(tr("No errors occurred, the packages were successfully installed."),true);
    FREELIST(data);

    return ALPM_ERR_OK;
}

int Alpm::fnmatch_cmp(const void *pattern, const void *string) {
    return fnmatch((const char *)pattern,(const char *)string,0);
}

int Alpm::remove_packages(const QList<AlpmPackage *> & m_pkgs,bool remove_cascade) {
    m_percent = -1;
    prev_event_type = -1;
    QString err;

    HandleReleaser handle_releaser(m_alpm_handle);

    if (alpm_trans_init(m_alpm_handle,remove_cascade?ALPM_TRANS_FLAG_RECURSE|ALPM_TRANS_FLAG_CASCADE:0)) {
        emit_information(lastError());
        emit_error(lastError());
        return alpm_errno(m_alpm_handle);
    }

    for (int i=0;i<m_pkgs.count();i++) {
        QVector<AlpmPackage *> pkgs = localDB().findByPackageName(m_pkgs[i]->name());
        if (pkgs.count() <= 0) continue;
        if (alpm_remove_pkg(m_alpm_handle,pkgs[0]->handle())) {
            if(alpm_errno(m_alpm_handle) == ALPM_ERR_TRANS_DUP_TARGET) {
                emit_information(tr("Skipping target: %1").arg(m_pkgs[i]->name()));
                continue;
            }
            err = QString("%1: %2").arg(m_pkgs[i]->name()).arg(lastError());
            emit_information(err);
            emit_error(err);
            return alpm_errno(m_alpm_handle);
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
            } while(list.next());
            data = NULL;
            break;
        }
        default:
            emit_information(lastError());
            emit_error(lastError());
            FREELIST(data);
            break;
        }
        return alpm_errno(m_alpm_handle);
    }

    FREELIST(data);

    bool ok;
    AlpmList<alpm_pkg_t> list(alpm_trans_get_remove(m_alpm_handle),AlpmList<alpm_pkg_t>::ignorefree);
    do {
        if (list.isEmpty()) break;
        alpm_pkg_t *pkg = list.valuePtr();
        QString name = QString::fromLocal8Bit(alpm_pkg_get_name(pkg));
        if(alpm_list_find(config.holdpkgs,alpm_pkg_get_name(pkg),fnmatch_cmp)) {
            emit_question(tr("%1 is designated as a HoldPkg, Do you want to continue?").arg(name),&ok);
            if (!ok) {
                list.detach();
                emit_error(tr("User rejected the removal of %1 package!").arg(name));
                return alpm_errno(m_alpm_handle);
            }
        }
    } while(list.next());
    list.detach();

    QStringList install_targets;
    QStringList remove_targets;
    if (!do_process_targets(true,install_targets,remove_targets)) {
        emit_information(tr("No packages were removed because of user's refusal."),true);
        return USER_REFUSAL;
    }

    if(alpm_trans_commit(m_alpm_handle, &data)) {
        err = tr("Failed to commit transaction (%1)").arg(lastError());
        emit_information(err);
        emit_error(err);
        FREELIST(data);
        return alpm_errno(m_alpm_handle);
    }
    FREELIST(data);

    handle_releaser.setPkgs(install_targets,remove_targets);

    emit_information(tr("No errors occurred, the packages were successfully removed."),true);
    return ALPM_ERR_OK;
}

ThreadRun::RC Alpm::installPackages(const QList<AlpmPackage *> & pkgs,bool asdeps,const QList<AlpmPackage *> & forcedpkgs) {
    if (!isValid(true)) return ThreadRun::BAD;

    ThreadRun::RC rc = run<int>(m_alpm_errno,this,&Alpm::install_packages,pkgs,asdeps?ALPM_TRANS_FLAG_ALLDEPS:0,forcedpkgs);
    return (rc == ThreadRun::OK && m_alpm_errno != ALPM_ERR_OK)?((m_alpm_errno == USER_REFUSAL)?ThreadRun::TERMINATED:ThreadRun::BAD):rc;
}

ThreadRun::RC Alpm::removePackages(const QList<AlpmPackage *> & pkgs,bool cascade) {
    if (!isValid(true)) return ThreadRun::BAD;

    ThreadRun::RC rc = run<int>(m_alpm_errno,this,&Alpm::remove_packages,pkgs,cascade);
    return (rc == ThreadRun::OK && m_alpm_errno != ALPM_ERR_OK)?((m_alpm_errno == USER_REFUSAL)?ThreadRun::TERMINATED:ThreadRun::BAD):rc;
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
    const char * fn = alpm_option_get_lockfile(m_alpm_handle);
    if (fn == NULL) return QString();
    return QString::fromLocal8Bit(fn);
}

QString Alpm::dbDirPath() const {
    QString ret = QString::fromLocal8Bit(alpm_option_get_dbpath(m_alpm_handle));
    return (ret.endsWith(QDir::separator()))?ret.mid(0,ret.length()-1):ret;
}

QStringList Alpm::cacheDirs() const {
    if (!isValid(true)) return QStringList();

    QStringList ret;
    if (getuid() != 0) return (ret << QDir::tempPath());

    AlpmList<char> dirs(alpm_option_get_cachedirs(m_alpm_handle),AlpmList<char>::ignorefree);
    if (dirs.count() <= 0) return ret;

    do {
        ret.append(QString::fromLocal8Bit(dirs.valuePtr()));
    } while (dirs.next());
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
        foreach(QString dirFile, dir.entryList()) {
           dir.remove(dirFile);
        }
    }

    return true;
}

bool Alpm::pkgLessThan(AlpmPackage * pkg1,AlpmPackage * pkg2) {
    int ret;
    if ((ret = pkg1->name().compare(pkg2->name())) != 0) return (ret < 0);
    if ((ret = Alpm::pkg_vercmp(pkg1->version(),pkg2->version())) != 0) return (ret < 0);
    if (pkg1->repo() == "local") return true;
    if (pkg2->repo() == "local") return false;
    return (pkg1->repo().compare(pkg2->repo()) < 0);
}

QVector<AlpmPackage *> Alpm::findByFileName(const QString & filename) const {
    QVector<AlpmPackage *> ret;
    if (!isValid(true)) return ret;

    QVector<AlpmPackage *> found_local = localDB().findByFileName(filename.mid(1));
    int j;
    QList<bool> found_local_flags;
    for (j=0;j<found_local.count();j++) {
        found_local_flags.append(false);
    }
    QList<AlpmDB> dbs = ((Alpm *)this)->allSyncDBs();
    for (int i=0;i<dbs.count();i++) {
        for (j=0;j<found_local.count();j++) {
            QVector<AlpmPackage *> pkgs = dbs[i].findByPackageName(found_local.at(j)->name());
            if (pkgs.count() > 0) {
                found_local_flags[j] = true;
                for (int k=0;k<pkgs.count();k++) {
                    ret.append(pkgs[k]);
                }
            }
        }
    }
    for (j=0;j<found_local_flags.count();j++) {
        if (!found_local_flags[j]) ret.append(found_local.at(j));
    }

    return ret;
}

QVector<AlpmPackage *> Alpm::find(const QRegularExpression & expr) const {
    QVector<AlpmPackage *> ret;
    if (!isValid(true)) return ret;

    ret += localDB().find(expr);
    QList<AlpmDB> dbs = ((Alpm *)this)->allSyncDBs();
    for (int i=0;i<dbs.count();i++) {
        ret += dbs[i].find(expr);
    }

    std::sort(ret.begin(),ret.end(),pkgLessThan);

    return ret;
}

QStringList Alpm::repos() const {
    QStringList m_repos;
    QList<AlpmDB> repo_list = ((Alpm *)this)->allSyncDBs();
    for (int id=0;id<repo_list.count();id++) {
        m_repos.append(repo_list[id].name());
    }
    m_repos.append(localDB().name());
    return m_repos;
}

QStringList Alpm::groups() const {
    return m_groups;
}

int Alpm::pkg_vercmp(const QString & ver1, const QString & ver2) {
    return ::alpm_pkg_vercmp(ver1.toLatin1().constData(),ver2.toLatin1().constData());
}
