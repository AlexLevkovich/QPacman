/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanCacheCleaner_H
#define PacmanCacheCleaner_H

#include "pacmanprocessreader.h"

class PacmanCacheCleaner : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanCacheCleaner(QObject *parent = 0);

protected:
    QByteArray command() const;
    void send_parameters();
};

#endif // PacmanCacheCleaner_H
