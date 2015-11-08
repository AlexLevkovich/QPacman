/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanRepositoryReader_H
#define PacmanRepositoryReader_H

#include "pacmanprocessreader.h"
#include "pacmanentry.h"

class PacmanRepositoryReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanRepositoryReader(QObject *parent = 0);

protected:
    void send_parameters();
    QByteArray command() const;

signals:
    void read_package(const PacmanEntry & package);
};

#endif // PacmanRepositoryReader_H
