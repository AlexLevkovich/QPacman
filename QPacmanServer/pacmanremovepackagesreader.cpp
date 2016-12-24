/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanremovepackagesreader.h"
#include "byteshumanizer.h"

#define TOTAL_REMOVED_STR "Total Removed Size:"

PacmanRemovePackagesReader::PacmanRemovePackagesReader(const QString & packages,bool withDeps,QObject *parent) : PacmanProcessReader(parent) {
    in_packages = packages;
    packagesRead = false;
    packagesWasRead = false;
    removing_wait = false;
    m_withDeps = withDeps;
    removeCanceled = false;
}

QString PacmanRemovePackagesReader::command() const {
    return QString(m_withDeps?"%2 -Rcs --noprogressbar %1":"%2 -Rc --noprogressbar %1").arg(in_packages).arg(PACMAN_BIN);
}

double PacmanRemovePackagesReader::total_removed() {
    return m_total_removed;
}

bool PacmanRemovePackagesReader::output(const QString & out) {
    QString line = out.simplified();
    if (line.isEmpty()) {
        if (packagesRead) packagesRead = false;
        return true;
    }

    if (packagesRead || line.startsWith("Packages (")) {
        int startindex = 0;
        if (!packagesRead) {
            packagesRead = true;
            packagesWasRead = true;
            startindex = 2;
        }
        QStringList parts = line.split(" ",QString::SkipEmptyParts);
        for (int i=startindex;i<parts.count();i++) {
             m_packages.append(parts[i]);
        }
    }

    if (packagesWasRead && line.startsWith(TOTAL_REMOVED_STR)) m_total_removed = BytesHumanizer(line.mid(strlen(TOTAL_REMOVED_STR)).trimmed()).value();

    if (!line.startsWith("removing ") || !line.endsWith("...")) {
        if (!current_removing.isEmpty()) {
            m_messages[current_removing].append(line);
        }
        return true;
    }

    QStringList parts = line.split(" ",QString::SkipEmptyParts);
    if (parts.count() < 2) return true;

    if (!current_removing.isEmpty() && m_messages.contains(current_removing)) {
        emit post_messages(current_removing,m_messages[current_removing]);
    }

    current_removing = parts[1].left(parts[1].length()-3);
    emit start_removing(current_removing);
    return true;
}

QStringList PacmanRemovePackagesReader::packages() const {
    return m_packages;
}

void PacmanRemovePackagesReader::onFinished(int code,QProcess::ExitStatus status) {
    if (!current_removing.isEmpty() && m_messages.contains(current_removing)) {
        emit post_messages(current_removing,m_messages[current_removing]);
    }

    if (packagesWasRead && (this->code() != 0)) {
        setCode(-code);
    }

    if (removeCanceled) setTerminated(true);

    PacmanProcessReader::onFinished(code,status);
}

bool PacmanRemovePackagesReader::error(const QString & error) {
    int index = error.indexOf(":: ");
    if (index != -1) {
        int index2 = error.indexOf("[Y/n]",0,Qt::CaseInsensitive);
        if (index2 != -1) {
            QString line = error.mid(index);
            if (line.startsWith(":: Do you want to remove these packages? [Y/n]")) {
                removing_wait = true;
                emit ready_to_process(m_packages.count());
                return true;
            }
            else {
                if (write("y\n") == -1) {
                    setCode(1);
                } else waitForBytesWritten();
                return true;
            }
        }
        return false;
    }
    return true;
}

void PacmanRemovePackagesReader::beginRemove() {
    if (!removing_wait) return;
    if (write("y\n") == -1) {
        setCode(1);
        return;
    }
    waitForBytesWritten();
}

void PacmanRemovePackagesReader::cancelRemove() {
    if (!removing_wait) return;
    if (write("n\n") == -1) {
        setCode(1);
        return;
    }
    removeCanceled = true;
    waitForBytesWritten();
}
