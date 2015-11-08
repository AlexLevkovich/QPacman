/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANINSTALLLOCALPACKAGESREADER_H
#define PACMANINSTALLLOCALPACKAGESREADER_H

#include "pacmaninstallpackagesreader.h"

class PacmanInstallLocalPackagesReader : public PacmanInstallPackagesReader {
    Q_OBJECT
public:
    explicit PacmanInstallLocalPackagesReader(const QStringList & packages,QObject *parent = 0);
    static const QString namesToString(const QStringList & packages);
    QStringList local_install_packages() const;

protected slots:
    void on_readyToProcess(int cnt);

protected:
    QString command() const;

private:
    QStringList m_local_install_packages;
    QStringList packages_input_list;
};

#endif // PACMANINSTALLLOCALPACKAGESREADER_H
