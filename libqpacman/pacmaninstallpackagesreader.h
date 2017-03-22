/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanInstallPackagesReader_H
#define PacmanInstallPackagesReader_H

#include "pacmanupdatepackagesreader.h"
#include <QMap>
#include "libqpacman_global.h"

class LIBQPACMANSHARED_EXPORT PacmanInstallPackagesReader : public PacmanUpdatePackagesReader {
    Q_OBJECT
public:
    explicit PacmanInstallPackagesReader(const QString & su_password,const QString & packages,QObject *parent = 0);
protected:
    virtual QString command() const;

protected:
    QString in_packages;
};

#endif // PacmanInstallPackagesReader_H
