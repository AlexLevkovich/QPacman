/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanRepositoryReader_H
#define PacmanRepositoryReader_H

#include "pacmanprocessreader.h"
#include "pacmanentry.h"

class LIBQPACMANSHARED_EXPORT PacmanRepositoryReader : public PacmanProcessReader {
    Q_OBJECT
public:
    PacmanRepositoryReader(QObject *parent = 0);
    ~PacmanRepositoryReader();

protected:
    virtual QString command() const;
    bool output(const QString & out);

signals:
    void read_package(PacmanEntry * package);

private:
    PacmanEntry * m_package;
};

#endif // PacmanRepositoryReader_H
