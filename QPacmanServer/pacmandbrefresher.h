/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANDBREFRESHER_H
#define PACMANDBREFRESHER_H

#include "pacmanprocessreader.h"

class PacmanDBRefresher : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanDBRefresher(QObject *parent = 0);
    ~PacmanDBRefresher();

protected:
    QString command() const;

protected slots:
    void start();

private:
    QStringList m_errors;
    QString tempConf;
};

#endif // PACMANDBREFRESHER_H
