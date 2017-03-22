#include <QStringList>
#include <QString>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <unistd.h>
#include <iostream>

using namespace std;

#define PACMAN_CACHEDIRS_MARKER "Cache Dirs: "

static const QStringList pacmanCacheDirs() {
    QProcess pacman_process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","C");
    pacman_process.setProcessEnvironment(env);
    pacman_process.start(PACMAN_BIN,QStringList()<<"-v");
    pacman_process.waitForFinished(-1);
    QStringList lines = QString::fromLocal8Bit(pacman_process.readAllStandardOutput()).split('\n');
    for (int i=0;i<lines.count();i++) {
        QString line = lines.at(i);
        if (!line.startsWith(PACMAN_CACHEDIRS_MARKER)) continue;
        return line.mid(::strlen(PACMAN_CACHEDIRS_MARKER)).split("  ",QString::SkipEmptyParts);
    }
    return QStringList();
}

int main() {
    QStringList dirs = pacmanCacheDirs();
    if (dirs.isEmpty()) {
        cerr << "cannot find pacman's cache dir pathes!!!\n";
        return 1;
    }

    if (setuid(0) < 0) {
        cerr << "setuid(0) execution was failed!!!\n";
        return 1;
    }

    for (int i=0;i<dirs.count();i++) {
        QDir dir(dirs.at(i));
        QStringList files = dir.entryList();
        for (int j=0;j<files.count();j++) {
            QFile(dir.path()+QDir::separator()+files.at(j)).remove();
        }
    }

    return 0;
}
