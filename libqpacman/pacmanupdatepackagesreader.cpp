/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanupdatepackagesreader.h"
#include "pacmansetupinforeader.h"
#include "pacmanentry.h"
#include "byteshumanizer.h"
#include <QDebug>
#include <QFile>
#include <QDate>
#include <QStringList>
#include "confsettings.h"

const QString PacmanUpdatePackagesReader::providerChooserStr("Enter a number (default=");
#define TOTAL_INSTALLED_STR "Total Installed Size:"
#define TOTAL_REMOVED_STR "Total Removed Size:"
#define RETRIEVING_PKGS_STR ":: Retrieving packages"

PacmanUpdatePackagesReader::PacmanUpdatePackagesReader(const QString & su_password,QObject *parent) : PacmanProcessReader(su_password,parent) {
    packagesRead = false;
    install_wait = false;
    packagesWasRead = false;
    m_total_installed = 0.0;
    m_total_removed = 0.0;
    packagesRetrieving = false;
    installCanceled = false;

    tempConf = ConfSettings::createTempConfName();

    setDebugOutput(true);
}

PacmanUpdatePackagesReader::~PacmanUpdatePackagesReader() {
    QFile::remove(tempConf);
}

void PacmanUpdatePackagesReader::start() {
    ConfSettings settings(tempConf);
    if (!settings.copyFromPacmanConf()) {
        setCode(1);
        addToErrorStreamCache(tr("Cannot copy pacman.conf to %1").arg(tempConf) + "\n");
    }
    settings.replaceXferCommand();
    PacmanProcessReader::start();
}

QString PacmanUpdatePackagesReader::command() const {
    return QString("%2 --config %1 --noprogressbar -Su").arg(tempConf).arg(PACMAN_BIN);
}

double PacmanUpdatePackagesReader::total_installed() {
    return m_total_installed;
}

double PacmanUpdatePackagesReader::total_removed() {
    return m_total_removed;
}

bool PacmanUpdatePackagesReader::output(const QString & out) {
    QString name;
    QString version;
    QString line = out.simplified();

    if (line.startsWith(":: ")) {
        if (!line.startsWith(RETRIEVING_PKGS_STR)) m_outErrors += line + "\n";
    }

    if (!packagesRead && line.startsWith("Packages (")) {
        int startindex = 2;
        packagesRead = true;
        packagesWasRead = true;

        QStringList parts = line.split(" ",QString::SkipEmptyParts);
        for (int i=startindex;i<parts.count();i++) {
            if (parts[i] == "[removal]") {
                PacmanEntry::parseNameVersion(m_install_packages[m_install_packages.count()-1],name,version);
                m_remove_packages.append(name+"-"+version);
                m_install_packages.removeAt(m_install_packages.count()-1);
            }
            else {
                PacmanEntry::parseNameVersion(parts[i],name,version);
                m_install_packages.append(name+"="+version);
            }
        }
    }

    if (install_wait && line.startsWith("checking package integrity...")) {
        packagesRetrieving = false;
        emit all_downloads_completed();
    }
    if (install_wait && (packagesRetrieving || line.startsWith(RETRIEVING_PKGS_STR))) packagesRetrieving = true;

    if (!packagesRead && !install_wait) {
        for (int i=1;;i++) {
            QString search_str = QString("%1) ").arg(i);
            int index = line.indexOf(search_str);
            if (index < 0) break;
            line = line.mid(index+search_str.length());
            int index2 = line.indexOf(' ',index+search_str.length());
            if (index2 < 0) {
                currentProviders[line] = i;
                break;
            }
            else {
                currentProviders[line.mid(0,index2)] = i;
                line = line.mid(index2);
            }
        }
    }

    if (packagesWasRead) {
        if (line.startsWith(TOTAL_INSTALLED_STR)) m_total_installed = BytesHumanizer(line.mid(strlen(TOTAL_INSTALLED_STR)).trimmed()).value();
        if (line.startsWith(TOTAL_REMOVED_STR)) m_total_removed = BytesHumanizer(line.mid(strlen(TOTAL_REMOVED_STR)).trimmed()).value();
    }

    if ((!line.startsWith("reinstalling ") &&
         !line.startsWith("installing ") &&
         !line.startsWith("upgrading ") &&
         !line.startsWith("removing ")) ||
         !line.endsWith("...")) {
        if (!current_installing.isEmpty()) {
            m_messages[current_installing].append(line);
        }
        return true;
    }

    QStringList parts = line.split(" ",QString::SkipEmptyParts);
    if (parts.count() < 2) return true;

    if (!current_installing.isEmpty() && m_messages.contains(current_installing) && !isDebugOutput()) {
        emit post_messages(current_installing,m_messages[current_installing]);
    }

    current_installing = parts[1].left(parts[1].length()-3);
    if (!parts[0].startsWith("removing")) emit start_installing(current_installing);
    else emit start_removing(current_installing);
    return true;
}

QStringList PacmanUpdatePackagesReader::install_packages() const {
    return m_install_packages;
}

QStringList PacmanUpdatePackagesReader::remove_packages() const {
    return m_remove_packages;
}

void PacmanUpdatePackagesReader::onFinished(int code,QProcess::ExitStatus status) {
    if (!current_installing.isEmpty() && m_messages.contains(current_installing) && !isDebugOutput()) {
        emit post_messages(current_installing,m_messages[current_installing]);
    }
    if (isDebugOutput()) emit post_messages("",((PacmanProcessReader *)this)->errorStream().split('\n'));

    if (installCanceled) setTerminated(true);

    PacmanProcessReader::onFinished(code,status);

    if (!m_outErrors.isEmpty()) addToErrorStreamCache(m_outErrors);
}

bool PacmanUpdatePackagesReader::error(const QString & error) {
    int index = -1;

    if (error.startsWith(":: ")) {
        int index2 = error.indexOf("[Y/n]",0,Qt::CaseInsensitive);
        if (index2 != -1) {
            if (error.startsWith(":: Proceed with installation? [Y/n]")) {
                waitForEmptyOutput();
                install_wait = true;
                emit ready_to_process(m_total_installed,m_total_removed);
                return true;
            }
            else {
                emit question_available((!warnings.isEmpty()?warnings:"")+error.mid(3,index2-3).trimmed());
                if (!warnings.isEmpty()) warnings.clear();
                return true;
            }
        }
        return false;
    }

    if (error.startsWith(providerChooserStr)) {
        waitForEmptyOutput();
        if (currentProviders.count() > 0) {
            emit some_providers_available(currentProviders.keys());
        }
        return true;
    }

    if (error.startsWith("warning: ")) {
        warnings += error + "\n";
        return true;
    }

    if (packagesRetrieving) {
        QString line;
        if (error.startsWith("--"+QDate::currentDate().toString(Qt::ISODate)+" ")) {
            line = error.mid(2);
            index = line.indexOf("--");
            if (index != -1) {
                line = line.mid(index+2).trimmed();
                if (!line.isEmpty()) {
                    emit start_download(line);
                }
            }
            return true;
        }

        if (error.startsWith("Length: ")) {
            line = error.mid(8);
            int index2 = line.indexOf(" ");
            if (index2 != -1) {
                bool ok = false;
                int ret_int = line.left(index2).toInt(&ok);
                if (ok) emit contents_length_found(ret_int);
            }
            return true;
        }

        index = error.indexOf("%[");
        if (index != -1) {
            int index2 = error.lastIndexOf(" ",index);
            if (index2 != -1) {
                bool ok = false;
                int ret_int = error.mid(index2+1,index-index2-1).toInt(&ok);
                if (ok) {
                    emit download_progress(ret_int);
                }
            }
            return true;
        }

        index = error.indexOf("wget: ");
        if (index != -1) {
            line = error.mid(index+6);
            if (!line.isEmpty()) {
                m_outErrors += line + "\n";
                setCode(1);
            }
            return true;
        }
    }

    return true;
}

void PacmanUpdatePackagesReader::beginInstall() {
    if (!install_wait) return;
    if (write("y\n") == -1) {
        setCode(1);
        return;
    }
    waitForBytesWritten();
}

void PacmanUpdatePackagesReader::cancelInstall() {
    if (!install_wait) return;
    if (write("n\n") == -1) {
        setCode(1);
        return;
    }
    installCanceled = true;
    waitForBytesWritten();
}

void PacmanUpdatePackagesReader::sendAnswer(char answer) {
    if (write(QByteArray(1,answer)+'\n') == -1) {
        setCode(1);
        return;
    }
    waitForBytesWritten();
}

void PacmanUpdatePackagesReader::sendChosenProvider(const QString & provider) {
    if (write(QString("%1\n").arg(currentProviders[provider]).toLocal8Bit()) == -1) {
        setCode(1);
        return;
    }
    waitForBytesWritten();
    currentProviders.clear();
}

