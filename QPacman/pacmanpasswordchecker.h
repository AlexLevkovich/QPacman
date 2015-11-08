/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANPASSWORDCHECKER_H
#define PACMANPASSWORDCHECKER_H

#include "pacmanprocessreader.h"

class PacmanPasswordChecker : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanPasswordChecker(const QByteArray & password,QObject *parent = 0);

protected:
    void send_parameters();
    QByteArray command() const;

protected slots:
    void error(const QString & error);

private:
    QByteArray m_password;
};

#endif // PACMANPASSWORDCHECKER_H
