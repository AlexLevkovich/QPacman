/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmaninstalllocalpackagesreader.h"
#include "pacmanentry.h"

PacmanInstallLocalPackagesReader::PacmanInstallLocalPackagesReader(const QString & su_password,const QStringList & packages,QObject *parent) : PacmanInstallPackagesReader(su_password,PacmanInstallLocalPackagesReader::namesToString(packages),parent) {
    packages_input_list = packages;
    connect(this,SIGNAL(ready_to_process(int)),this,SLOT(on_readyToProcess(int)));
}

QString PacmanInstallLocalPackagesReader::command() const {
    return QString("%3 --config %2 -U --noprogressbar %1").arg(in_packages).arg(tempConf).arg(PACMAN_BIN);
}

void PacmanInstallLocalPackagesReader::on_readyToProcess(int /*cnt*/) {
    QString name;
    QString version;
    QString package_name;
    int index;

    for (int i=0;i<packages_input_list.count();i++) {
        index = packages_input_list[i].lastIndexOf('-');
        if (index == -1) continue;
        PacmanEntry::parseNameVersion(packages_input_list[i].left(index),name,version);
        package_name = name + "=" + version;
        index = m_install_packages.indexOf(package_name);
        if (index == -1) continue;
        m_local_install_packages.append(m_install_packages[index]);
        m_install_packages.removeAt(index);
    }
}

const QString PacmanInstallLocalPackagesReader::namesToString(const QStringList & packages) {
    QString in_packages;
    for (int i=0;i<packages.count();i++) {
        in_packages += "\"" + packages[i] + "\" ";
    }

    return in_packages;
}

QStringList PacmanInstallLocalPackagesReader::local_install_packages() const {
    return m_local_install_packages;
}
