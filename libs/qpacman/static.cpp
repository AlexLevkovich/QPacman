/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "static.h"
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#include <QList>
#include <QProcess>
#include <QFileInfo>
#include <QDebug>
#include <QDirIterator>
#include <QTranslator>
#include <QLibraryInfo>
#include <QCryptographicHash>
#include "windowcenterer.h"
#include "libalpm.h"

QString Static::Error_Str;
QString Static::Warning_Str;
QString Static::Question_Str;
QString Static::Info_Str;
QString Static::InstalledSuccess_Str;
QString Static::PostMessages_Str;
QString Static::TransTerminate_Str;
QString Static::RepoAll_Str;
QString Static::QPacman_Key = "QPacmanApp";

Static::Static() {
    init_tr_variables();
}

void Static::init_tr_variables() {
    RepoAll_Str = QObject::tr("All");
    Error_Str = QObject::tr("Error...");
    Warning_Str = QObject::tr("Warning...");
    Info_Str = QObject::tr("Information...");
    Question_Str = QObject::tr("Question...");
    InstalledSuccess_Str = QObject::tr("The packages were installed successfully!");
    PostMessages_Str = QObject::tr("Post messages for %1 package...");
    TransTerminate_Str = QObject::tr("It is not good idea to terminate the current transaction.\nAre you sure?");
}

SingleApplication * Static::pApp() {
    if (QCoreApplication::instance()->inherits("SingleApplication")) return (SingleApplication *)QCoreApplication::instance();
    return NULL;
}

const QList<QAction *> Static::childrenActions(QObject * main_object) {
    QList<QAction *> actions;
    QObjectList listObjects = main_object->children();
    for (int i=0;i<listObjects.count();i++) {
        actions.append(childrenActions(listObjects[i]));
        if (listObjects[i]->inherits("QAction")) actions.append((QAction *)listObjects[i]);
    }

    return actions;
}

void Static::makeCentered(QWidget * wnd) {
    if (wnd == NULL) return;

    new WindowCenterer(wnd);
}

const QString Static::fixButtonText(const QString & label) {
    QString fixedLabel;
    int i = 0;
    int lastIndex = label.length() - 1;
    while(i < lastIndex) {
        if (label[i] == '&') {
            if (label[i + 1] == '&') {
                fixedLabel += label[i];
                ++i;
            }
        }
        else fixedLabel += label[i];
        ++i;
    }
    if (label[lastIndex] != '&') fixedLabel += label[lastIndex];

    return fixedLabel;
}

void Static::setIniValue(const QString & key,const QVariant & value) {
    QSettings settings;
    settings.setValue("settings/"+key,value);
}

int Static::startUnderRoot(int argc, char *argv[]) {
    QProcess process;
    process.setProgram((QFileInfo(QString::fromLocal8Bit(argv[0])).dir() == QFileInfo(QString::fromLocal8Bit(QSU_APP2)).dir())?QString::fromLocal8Bit(QSU_APP2):QString::fromLocal8Bit(QSU_APP1));
    QStringList args;
    for (int i=0;i<argc;i++) {
        args << QString::fromLocal8Bit(argv[i]);
    }
    process.setArguments(args);
    process.start();
    while (process.state() != QProcess::Running && process.error() != QProcess::FailedToStart) {
        if (process.waitForStarted(100)) break;
    }
    if (process.state() != QProcess::Running) return 127;
    process.waitForFinished(-1);
    return process.exitCode();
}

bool Static::startQPacman(const QStringList & args,QObject * receiver,const char * finished_slot,const QString & output_file_name) {
    QProcess * process = new QProcess();
    if (!output_file_name.isEmpty()) {
        process->setProcessChannelMode(QProcess::MergedChannels);
        process->setStandardOutputFile(output_file_name);
    }
    process->setProgram((QDir(QCoreApplication::applicationDirPath()) == QFileInfo(QString::fromLocal8Bit(QPACMAN_APP2)).dir())?QString::fromLocal8Bit(QPACMAN_APP2):QString::fromLocal8Bit(QPACMAN_APP1));
    process->setArguments(args);
    if (receiver != NULL) QObject::connect(process,SIGNAL(finished(int, QProcess::ExitStatus)),receiver,finished_slot);
    QObject::connect(process,SIGNAL(finished(int, QProcess::ExitStatus)),process,SLOT(deleteLater()));
    process->start();
    while (process->state() != QProcess::Running && process->error() != QProcess::FailedToStart) {
        if (process->waitForStarted(100)) break;
    }
    if (process->state() != QProcess::Running) {
        delete process;
        return false;
    }
    return true;
}

const QStringList Static::dirContents(const QDir & dir,const QString & nameFilter) {
    QStringList ret;
    QDirIterator it(dir.path(),QStringList() << nameFilter);
    while (it.hasNext()) {
        ret << it.next();
    }
    return ret;
}

int Static::have_file_with_same_name(const QStringList & arr,const QString & name) {
    if (arr.count() <= 0) return -1;

    for (int i=0;i<arr.count();i++) {
        if (QFileInfo(arr.at(i)).fileName() == name) return i;
    }

    return -1;
}

qint64 Static::compare_file_dates(const QString & left,const QString & right) {
    return QFileInfo(left).fileTime(QFileDevice::FileModificationTime).toMSecsSinceEpoch() - QFileInfo(right).fileTime(QFileDevice::FileModificationTime).toMSecsSinceEpoch();
}

int Static::compare_file_sizes(const QString & left,const QString & right) {
    return md5_summ(left).compare(md5_summ(right));
}

bool Static::isLeftDirNewer(const QDir & left,const QDir & right) {
    QString ext = AlpmDB::extension();
    QStringList left_files = leave_only_real_db_files(dirContents(left,"*"+ext));
    QStringList right_files = leave_only_real_db_files(dirContents(right,"*"+ext));
    if (Alpm::instance() == NULL) return false;

    QList<AlpmDB> dbs = Alpm::instance()->allSyncDBs();
    QStringList expected_names;
    int i;
    for (i=0;i<dbs.count();i++) {
        expected_names.append(dbs[i].name()+ext);
    }

    QList<int> left_files_idxs;
    QList<int> right_files_idxs;
    int idx;
    for (i=0;i<expected_names.count();i++) {
        if ((idx = have_file_with_same_name(left_files,expected_names.at(i))) < 0) return false;
        left_files_idxs.append(idx);
        if ((idx = have_file_with_same_name(right_files,expected_names.at(i))) < 0) return false;
        right_files_idxs.append(idx);
    }

    bool same = true;
    for (i=(left_files_idxs.count()-1);i>=0;i--) {
        if (compare_file_sizes(left_files.at(left_files_idxs.at(i)),right_files.at(right_files_idxs.at(i))) != 0) {
            same = false;
        }
        else {
            left_files_idxs.removeAt(i);
            right_files_idxs.removeAt(i);
        }
    }
    if (same) return false;

    for (i=0;i<left_files_idxs.count();i++) {
        if (compare_file_dates(left_files.at(left_files_idxs.at(i)),right_files.at(right_files_idxs.at(i))) > 0) {
            return true;
        }
    }

    return false;
}

const QStringList Static::leave_only_real_db_files(const QStringList & list) {
    if (!Alpm::isOpen()) return QStringList();

    QStringList ret_list = list;
    QStringList db_names;
    QList<AlpmDB> list_dbs = Alpm::instance()->allSyncDBs();
    int i;
    QString ext = AlpmDB::extension();
    for (i=0;i<list_dbs.count();i++) {
        db_names << list_dbs[i].name()+ext;
    }
    for (i=(list.count()-1);i>=0;i--) {
        if (!db_names.contains(QFileInfo(list.at(i)).fileName())) ret_list.removeAt(i);
    }
    return ret_list;
}

const QByteArray Static::md5_summ(const QString & filepath) {
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) return QByteArray();

    hash.addData(&file);
    return hash.result();
}

void Static::runDetachedUnderOrigUser(const QString & program,const QStringList & args) {
    QProcess process;
    if (getuid() == 0) {
        process.setProgram(SU_BIN);
        process.setArguments(QStringList() << "-" << getenv("ORIGINAL_USER") << "-c" << QString("DISPLAY=%1 %2 %3").arg(getenv("ORIGINAL_DISPLAY")).arg(program).arg(args.join(" ")));
    }
    else {
        process.setProgram(program);
        process.setArguments(args);
    }
    process.startDetached();
}

const QSize Static::quadroSize(int dimension) {
    QSize size(dimension,dimension);
    return size;
}

void Static::setupTranslations(const QString & mainName,const QDir & installDir,const QDir & mainLocalDir,const QDir & alpmLocalDir,const QDir & libLocalDir) {
    QTranslator * m_translator = new QTranslator(QCoreApplication::instance());
    if(!m_translator->load(QLocale::system(),mainName,"_",mainLocalDir.path())) m_translator->load(QLocale::system(),mainName,"_",installDir.path());
    QCoreApplication::installTranslator(m_translator);
    QTranslator * m_translator2 = new QTranslator(QCoreApplication::instance());
    if (m_translator2->load(QLocale::system(),"qt","_",QLibraryInfo::location(QLibraryInfo::TranslationsPath))) QCoreApplication::installTranslator(m_translator2);
    QTranslator * m_translator3 = new QTranslator(QCoreApplication::instance());
    if(!m_translator3->load(QLocale::system(),"libqpacman","_",libLocalDir.path())) m_translator3->load(QLocale::system(),"libqpacman","_",installDir.path());
    QCoreApplication::installTranslator(m_translator3);
    QTranslator * m_translator4 = new QTranslator(QCoreApplication::instance());
    if(!m_translator4->load(QLocale::system(),"libqalpm","_",alpmLocalDir.path())) m_translator4->load(QLocale::system(),"libqalpm","_",installDir.path());
    QCoreApplication::installTranslator(m_translator4);
}
