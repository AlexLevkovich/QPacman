/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanFilePackageInfoReader_H
#define PacmanFilePackageInfoReader_H

#include "pacmanprocessreader.h"
#include "pacmanentry.h"
#include "libqpacman_global.h"

class LIBQPACMANSHARED_EXPORT PacmanFilePackageInfoReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanFilePackageInfoReader(const QString & package,QObject *parent = 0);
    PacmanEntry info() const { return entry; }

protected:
    QString command() const;
    bool error(const QString & err);
    bool output(const QString & out);

private:
    QString m_package;
    PacmanEntry entry;
    bool wasStderrRead;
};

#endif // PacmanFilePackageInfoReader_H