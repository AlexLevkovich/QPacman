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
    void send_parameters();
    QByteArray command() const;

private slots:
    void on_package_ready(const PacmanEntry & entry);

private:
    QString m_package;
    PacmanEntry entry;
};

#endif // PacmanFilePackageInfoReader_H
