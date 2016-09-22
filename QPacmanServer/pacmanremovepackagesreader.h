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
    explicit PacmanRemovePackagesReader(const QString & packages,bool withDeps = true,QObject *parent = 0);
    QStringList packages() const;
    void beginRemove();
    void cancelRemove();
    double total_removed();

protected:
    QString command() const;
    bool error(const QString & error);
    bool output(const QString & out);

protected slots:
    void onFinished(int code,QProcess::ExitStatus status);

signals:
    void start_removing(const QString & name);
    void post_messages(const QString & package_name,const QStringList & messages);
    void ready_to_process(int cnt);

protected:
    QString in_packages;

private:
    QStringList m_packages;
    bool removing_wait;
    bool packagesRead;
    bool packagesWasRead;
    QString current_removing;
    QMap<QString,QStringList> m_messages;
    double m_total_removed;
    bool m_withDeps;
    QString errorStream;
    bool removeCanceled;
};

#endif // PacmanRemovePackagesReader_H
