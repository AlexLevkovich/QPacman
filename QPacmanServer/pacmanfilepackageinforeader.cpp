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
    return QString("%2/stdbuf -i0 -o0 -e0 %2/tar -xpvf \"%1\" .PKGINFO -O").arg(m_package).arg(TOOLS_BIN);
}

void PacmanFilePackageInfoReader::readyReadStandardOutput() {
    wasStdoutRead = true;
    while (process.canReadLine()) {
        QByteArray line = process.readLine();
        if (!line.simplified().isEmpty()) entry.parseLine(line);
    }
}

void PacmanFilePackageInfoReader::error(const QString & error) {
    if (error.startsWith(".PKGINFO")) {
        if (!wasStdoutRead) {
            process.waitForReadyRead();
        }
        process.terminate();
        m_errorStream.clear();
        code = 0;
        onFinished(0,QProcess::NormalExit);
    }
    else code = 1;
}
