/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANINSTALLLOCALPACKAGESREADER_H
#define PACMANINSTALLLOCALPACKAGESREADER_H

#include "pacmaninstallpackagesreader.h"
#include <QStringList>

class PacmanInstallLocalPackagesReader : public PacmanInstallPackagesReader {
    Q_OBJECT
public:
    explicit PacmanInstallLocalPackagesReader(const QStringList & packages,QObject *parent = 0);
    QStringList local_install_packages() const;

protected:
    void send_parameters();
    QByteArray command() const;

protected slots:
    virtual void on_readyToProcess(const QStringList & install_packages,const QStringList & remove_packages,const QStringList & local_install_packages,qreal total_installed, qreal total_removed);

private:
    QStringList m_packages;
    QStringList m_local_install_packages;
};

#endif // PACMANINSTALLLOCALPACKAGESREADER_H
