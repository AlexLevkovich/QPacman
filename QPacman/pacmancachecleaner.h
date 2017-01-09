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
    explicit PacmanCacheCleaner(const QString & su_password,QObject *parent = 0);

protected:
    QString command() const;
};

#endif // PacmanCacheCleaner_H
