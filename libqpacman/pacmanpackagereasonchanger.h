/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANPACKAGEREASONCHANGER_H
#define PACMANPACKAGEREASONCHANGER_H

#include "pacmanprocessreader.h"

class LIBQPACMANSHARED_EXPORT PacmanPackageReasonChanger : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanPackageReasonChanger(const QString & su_password,const QString & package,bool doDeps,QObject *parent = 0);

protected:
    QString command() const;

private:
    QString m_package;
    bool m_doDeps;
};

#endif // PACMANPACKAGEREASONCHANGER_H