/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANDBREFRESHER_H
#define PACMANDBREFRESHER_H

#include "pacmanprocessreader.h"
#include "libqpacman_global.h"

class LIBQPACMANSHARED_EXPORT PacmanDBRefresher : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanDBRefresher(const QString & su_password,QObject *parent = 0);
    ~PacmanDBRefresher();

protected:
    QString command() const;
    inline bool showErrorMessageAtExit() {
        return false;
    }

protected slots:
    void start();

private:
    QString tempConf;
};

#endif // PACMANDBREFRESHER_H
