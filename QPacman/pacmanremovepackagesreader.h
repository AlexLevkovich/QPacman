/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanRemovePackagesReader_H
#define PacmanRemovePackagesReader_H

#include "pacmanprocessreader.h"
#include <QStringList>

class PacmanRemovePackagesReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanRemovePackagesReader(const QString & packages,QObject *parent = 0);
    QStringList packages() const;
    void beginRemove();
    void cancelRemove();

protected:
    void send_parameters();
    QByteArray command() const;

private slots:
    void on_readyToProcess(const QStringList & packages,qreal total_removed);

signals:
    void ready_to_process(qreal total_removed);
    void post_messages(const QString & package_name,const QStringList & messages);
    void start_removing(const QString & name);

private:
    QString in_packages;
    QStringList m_packages;
};

#endif // PacmanRemovePackagesReader_H
