/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanFilePackageInfoReader_H
#define PacmanFilePackageInfoReader_H

#include "pacmanprocessreader.h"
#include "pacmanentry.h"

class PacmanFilePackageInfoReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanFilePackageInfoReader(const QString & package,QObject *parent = 0);
    PacmanEntry info() const { return entry; }

protected:
    QString command() const;
    void error(const QString & error);

protected slots:
    void readyReadStandardOutput();

private:
    QString m_package;
    PacmanEntry entry;
    bool wasStdoutRead;
};

#endif // PacmanFilePackageInfoReader_H
