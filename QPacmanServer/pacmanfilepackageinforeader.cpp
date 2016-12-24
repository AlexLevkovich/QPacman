/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanfilepackageinforeader.h"

PacmanFilePackageInfoReader::PacmanFilePackageInfoReader(const QString & package,QObject *parent) : PacmanProcessReader(parent) {
    m_package = package;
    wasStdoutRead = false;
}

QString PacmanFilePackageInfoReader::command() const {
    return QString("%2 -i0 -o0 -e0 %3 -xpvf \"%1\" .PKGINFO -O").arg(m_package).arg(STDBUF_BIN).arg(TAR_BIN);
}

bool PacmanFilePackageInfoReader::output(const QString & line) {
    wasStdoutRead = true;
    if (!line.simplified().isEmpty()) entry.parseLine(line.toLocal8Bit());
    return true;
}

bool PacmanFilePackageInfoReader::error(const QString & err) {
    if (err.startsWith(".PKGINFO")) {
        if (!wasStdoutRead) {
            waitForReadyRead();
        }
        terminateProcess();
        clearErrorStreamCache();
        setCode(0);
        onFinished(0,QProcess::NormalExit);
    }
    else setCode(1);
    return true;
}
