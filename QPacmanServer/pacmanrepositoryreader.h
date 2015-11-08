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
    virtual QString command() const;

protected slots:
    virtual void readyReadStandardOutput();

signals:
    void read_package(const PacmanEntry & package);

private:
    PacmanEntry m_package;
};

#endif // PacmanRepositoryReader_H
