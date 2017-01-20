/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanfilepackageinforeader.h"

PacmanFilePackageInfoReader::PacmanFilePackageInfoReader(const QString & package,QObject *parent) : PacmanProcessReader(parent) {
    m_package = package;
    wasStderrRead = false;
}

QString PacmanFilePackageInfoReader::command() const {
    return QString("%2 -xpvf \"%1\" .PKGINFO -O").arg(m_package).arg(TAR_BIN);
}

bool PacmanFilePackageInfoReader::output(const QString & line) {
    if (!line.simplified().isEmpty()) entry.parseLine(line.toLocal8Bit());
    if (availableOutputBytesCount() <= 0) terminate();
    return true;
}

bool PacmanFilePackageInfoReader::error(const QString & err) {
    if (!wasStderrRead && !err.startsWith(".PKGINFO")) {
        terminate();
        setCode(1);
    }
    else wasStderrRead = true;
    return true;
}
