/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanFilesListReader_H
#define PacmanFilesListReader_H

#include "pacmanprocessreader.h"
#include <QMap>

class PacmanFilesListReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanFilesListReader(QObject *parent = 0);

protected:
    QString command() const;
    void error(const QString & error);

protected slots:
    void readyReadStandardOutput();
    void onFinished(int code,QProcess::ExitStatus status);

signals:
    void files_ready(const QString & package,const QStringList & files);

private:
    QString m_package;
    QStringList m_files;
    QString m_dir;
};

#endif // PacmanFilesListReader_H
