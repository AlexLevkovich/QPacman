/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanRemovePackagesReader_H
#define PacmanRemovePackagesReader_H

#include "pacmanprocessreader.h"
#include <QMap>

class PacmanRemovePackagesReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanRemovePackagesReader(const QString & packages,QObject *parent = 0);
    QStringList packages() const;
    void beginRemove();
    void cancelRemove();
    qreal total_removed();

protected:
    QString command() const;
    void error(const QString & error);

protected slots:
    void readyReadStandardError();
    void readyReadStandardOutput();
    void onFinished(int code,QProcess::ExitStatus status);

signals:
    void start_removing(const QString & name);
    void post_messages(const QString & package_name,const QStringList & messages);
    void ready_to_process(int cnt);

protected:
    QString in_packages;

private:
    QStringList m_packages;
    int countRead;
    bool removing_wait;
    bool packagesRead;
    bool packagesWasRead;
    QString current_removing;
    QMap<QString,QStringList> m_messages;
    qreal m_total_removed;
};

#endif // PacmanRemovePackagesReader_H
