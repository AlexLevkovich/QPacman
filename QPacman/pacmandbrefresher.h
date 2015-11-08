/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanDBRefresher_H
#define PacmanDBRefresher_H

#include "pacmanprocessreader.h"

class PacmanDBRefresher : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanDBRefresher(QObject *parent = 0);

protected:
    void send_parameters();
    QByteArray command() const;
};

#endif // PacmanDBRefresher_H
