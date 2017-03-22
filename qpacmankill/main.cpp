#include <QList>
#include <QStringList>
#include <QString>
#include <QProcess>
#include <QFileInfo>
#include <unistd.h>
#include <proc/readproc.h>
#include <sys/types.h>
#include <signal.h>
#include <iostream>

using namespace std;

#define PACMAN_LOCK_FILE_MARKER "Lock File : "

class Proc {
public:
    Proc(const proc_t & proc_info) {
        m_pid = proc_info.tid;
        m_ppid = proc_info.ppid;
        for (int i = 0;proc_info.cmdline != NULL && proc_info.cmdline[i] != NULL;i++) {
            if (i == 0) m_pgmname = QString::fromLocal8Bit(proc_info.cmdline[i]);
            else m_args.append(QString::fromLocal8Bit(proc_info.cmdline[i]));
        }
    }

    Proc(int pid,int ppid) {
        m_pid = pid;
        m_ppid = ppid;
    }

    int pid() const { return m_pid; }
    int ppid() const { return m_ppid; }
    QString pgmname() const { return m_pgmname; }
    QStringList args() const { return m_args; }

private:
    int m_pid;
    int m_ppid;
    QString m_pgmname;
    QStringList m_args;
};

static bool procLessThan(const Proc &proc1, const Proc &proc2) {
    return proc1.ppid() < proc2.ppid();
}

static bool procLessThan2(const Proc &proc1, const Proc &proc2) {
    return proc1.pid() < proc2.pid();
}

static QList<Proc> processes;
static QList<Proc> processes2;
static QList<int> kill_pids;

static void findChildren(int pid) {
    Proc search_pid(-1,pid);

    QList<Proc>::iterator begin = qLowerBound(processes.begin(),processes.end(),search_pid,procLessThan);
    QList<Proc>::iterator end = qUpperBound(processes.begin(),processes.end(),search_pid,procLessThan);

    QList<Proc>::iterator i = begin;
    while (i != end) {
        kill_pids.append(i->pid());
        findChildren(i->pid());
        ++i;
    }
}

static Proc * findSelf(int pid) {
    QList<Proc>::iterator index = qBinaryFind(processes2.begin(),processes2.end(),Proc(pid,-1),procLessThan2);
    if (index == processes2.end()) return NULL;

    return &(*index);
}

static Proc * findParent(int pid) {
    Proc * proc = findSelf(pid);
    if (proc == NULL) return NULL;

    return findSelf(proc->ppid());
}

static const QString pacmanLockFile() {
    QProcess pacman_process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","C");
    pacman_process.setProcessEnvironment(env);
    pacman_process.start(PACMAN_BIN,QStringList()<<"-v");
    pacman_process.waitForFinished(-1);
    QStringList lines = QString::fromLocal8Bit(pacman_process.readAllStandardOutput()).split('\n');
    for (int i=0;i<lines.count();i++) {
        QString line = lines.at(i);
        if (!line.startsWith(PACMAN_LOCK_FILE_MARKER)) continue;
        return line.mid(::strlen(PACMAN_LOCK_FILE_MARKER));
    }
    return "";
}

int main(int argc, char *argv[]) {
    char pgmname[1000];

    strcpy(pgmname,QFileInfo(QString::fromLocal8Bit(argv[0])).fileName().toLocal8Bit().constData());

    if (argc < 2) {
        cerr << "Usage: " << pgmname << " {pid}\n";
        return 1;
    }

    int pid = atoi(argv[1]);
    if (pid <= 0) {
        cerr << "incorrect pid=" << pid << " is passed!!!\n";
        return 1;
    }

    QString pacman_lock_file = pacmanLockFile();
    if (pacman_lock_file.isEmpty()) {
        cerr << "cannot find pacman's lock file path!!!\n";
        return 1;
    }

    if (setuid(0) < 0) {
        cerr << "setuid(0) execution was failed!!!\n";
        return 1;
    }

    PROCTAB* proc = ::openproc(PROC_FILLCOM | PROC_FILLSTAT | PROC_FILLSTATUS);
    proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_info));
    while (readproc(proc, &proc_info) != NULL) {
        processes.append(Proc(proc_info));
    }
    closeproc(proc);
    sort(processes.begin(),processes.end(),procLessThan);
    processes2=processes;
    sort(processes2.begin(),processes2.end(),procLessThan2);

    Proc * parent_proc = findParent(pid);
    if (parent_proc == NULL) {
        cerr << "parent of the passed pid is incorrect!!!\n";
        return 1;
    }

    if (!parent_proc->pgmname().endsWith("QPacman") &&
        !parent_proc->pgmname().endsWith("QPacmanTray")) {
        cerr << "passed pid has incorrect parent application!!!\n";
        return 1;
    }

    findChildren(pid);

    for (int i=(kill_pids.count()-1);i>=0;i--) {
        ::kill(kill_pids.at(i),SIGKILL);
    }
    ::kill(pid,SIGKILL);

    QFile(pacman_lock_file).remove();

    return 0;
}
