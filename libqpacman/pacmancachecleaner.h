/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanCacheCleaner_H
#define PacmanCacheCleaner_H

#include "pacmanprocessreader.h"
#include "libqpacman_global.h"

class LIBQPACMANSHARED_EXPORT PacmanCacheCleaner : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanCacheCleaner(QObject *parent = 0);

protected:
    QString command() const;
};

#endif // PacmanCacheCleaner_H
