/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmaninstallpackagesreader.h"
#include "pacmanentry.h"
#include "byteshumanizer.h"
#include <QDebug>
#include <QDate>
#include <QTextStream>
#include <QFile>
#include <QStringList>
#include "confsettings.h"

const QString PacmanInstallPackagesReader::providerChooserStr("Enter a number (default=");
#define TOTAL_INSTALLED_STR "Total Installed Size:"
#define TOTAL_REMOVED_STR "Total Removed Size:"
#define RETRIEVING_PKGS_STR ":: Retrieving packages"

PacmanInstallPackagesReader::PacmanInstallPackagesReader(const QString & packages,QObject *parent) : PacmanProcessReader(parent) {
    in_packages = packages;
    packagesRead = false;
    packages_count = 0;
    install_wait = false;
    packagesWasRead = false;
    m_total_installed = 0.0;
    m_total_removed = 0.0;
    packagesRetrieving = false;
    installCanceled = false;

    tempConf = ConfSettings::createTempConfName();
}

PacmanInstallPackagesReader::~PacmanInstallPackagesReader() {
    QFile::remove(tempConf);
}

void PacmanInstallPackagesReader::start() {
    ConfSettings settings(tempConf);
    if (!settings.copyFromPacmanConf()) {
        code = 1;
        m_errorStream += tr("Cannot copy pacman.conf to %1").arg(tempConf) + "\n";
    }
    settings.replaceXferCommand();

    PacmanProcessReader::start();
}

QString PacmanInstallPackagesReader::command() const {
    return QString("%2/stdbuf -i0 -o0 -e0 %2/pacman --config %3 -S --noprogressbar %1").arg(in_packages).arg(TOOLS_BIN).arg(tempConf);
}

double PacmanInstallPackagesReader::total_installed() {
    return m_total_installed;
}

double PacmanInstallPackagesReader::total_removed() {
    return m_total_removed;
}

void PacmanInstallPackagesReader::readyReadStandardOutput() {
    QString name;
    QString version;

    while (process.canReadLine()) {
        QString line = QString::fromLocal8Bit(process.readLine());

        line = line.simplified();
        if (line.isEmpty()) {
            if (packagesRead) packagesRead = false;
            continue;
        }

        if (line.startsWith(":: ")) {
            if (!line.startsWith(RETRIEVING_PKGS_STR)) m_outErrors += line + "\n";
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
                if (parts[i] == "[removal]") {
                    PacmanEntry::parseNameVersion(m_install_packages[m_install_packages.count()-1],name,version);
                    m_remove_packages.append(name+"-"+version);
                    m_install_packages.removeAt(m_install_packages.count()-1);
                }
                else {
                    PacmanEntry::parseNameVersion(parts[i],name,version);
                    m_install_packages.append(name+"="+version);
                }
                packages_count++;
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
            continue;
        }

        QStringList parts = line.split(" ",QString::SkipEmptyParts);
        if (parts.count() < 2) continue;

        if (!current_installing.isEmpty() && m_messages.contains(current_installing)) {
            emit post_messages(current_installing,m_messages[current_installing]);
        }

        current_installing = parts[1].left(parts[1].length()-3);
        if (!parts[0].startsWith("removing")) emit start_installing(current_installing);
        else emit start_removing(current_installing);
    }
}

QStringList PacmanInstallPackagesReader::install_packages() const {
    return m_install_packages;
}

QStringList PacmanInstallPackagesReader::remove_packages() const {
    return m_remove_packages;
}

void PacmanInstallPackagesReader::onFinished(int code,QProcess::ExitStatus status) {
    if (!current_installing.isEmpty() && m_messages.contains(current_installing)) {
        emit post_messages(current_installing,m_messages[current_installing]);
    }

    if (installCanceled) isTerminated = true;

    PacmanProcessReader::onFinished(code,status);

    if (!m_outErrors.isEmpty()) {
        m_errorStream += m_outErrors;
    }
}

void PacmanInstallPackagesReader::readyReadStandardError() {
    m_errorStream += QString::fromLocal8Bit(process.readAllStandardError());

    QStringList errorLines = m_errorStream.split("\n",QString::SkipEmptyParts);
    m_errorStream.clear();

    QString err;
    for (int i=0;i<(errorLines.count()-1);i++) {
        err = errorLines.at(i);
        err.remove('\r');
        error(err);
    }

    if (errorLines.count() > 0) {
        if (!error(errorLines.last())) m_errorStream += errorLines.last();
    }
}

bool PacmanInstallPackagesReader::error(const QString & error) {
    int index = -1;

    if (error.startsWith(":: ")) {
        int index2 = error.indexOf("[Y/n]",0,Qt::CaseInsensitive);
        if (index2 != -1) {
            if (error.startsWith(":: Proceed with installation? [Y/n]")) {
                install_wait = true;
                emit ready_to_process(packages_count);
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

    if (packagesRetrieving) {
        QString line;
        if (error.startsWith("--"+QDate::currentDate().toString(Qt::ISODate)+" ")) {
            line = error.mid(2);
            index = line.indexOf("--");
            if (index != -1) {
                line = line.mid(index+2);
                QTextStream stream(&line,QIODevice::ReadOnly);
                QString ret = stream.readLine().trimmed();
                if (!ret.isEmpty()) {
                    emit start_download(ret);
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
                code = 1;
            }
            return true;
        }
    }

    if (error.startsWith(providerChooserStr)) {
        if (currentProviders.count() > 0) {
            emit some_providers_available(currentProviders.keys());
        }
        return true;
    }

    if (error.startsWith("warning: ")) {
        warnings += error + "\n";
    }

    return true;
}

void PacmanInstallPackagesReader::beginInstall() {
    if (!install_wait) return;
    if (process.write("y\n") == -1) {
        code = 1;
        return;
    }
    process.waitForBytesWritten(-1);
}

void PacmanInstallPackagesReader::cancelInstall() {
    if (!install_wait) return;
    if (process.write("n\n") == -1) {
        code = 1;
        return;
    }
    installCanceled = true;
    process.waitForBytesWritten(-1);
}

void PacmanInstallPackagesReader::sendAnswer(int answer) {
    if (process.write(QByteArray(1,(char)answer)+"\n") == -1) {
        code = 1;
        return;
    }
    process.waitForBytesWritten(-1);
}

void PacmanInstallPackagesReader::sendChosenProvider(const QString & provider) {
    if (process.write(QString("%1\n").arg(currentProviders[provider]).toLocal8Bit()) == -1) {
        code = 1;
        return;
    }
    process.waitForBytesWritten(-1);
    currentProviders.clear();
}
