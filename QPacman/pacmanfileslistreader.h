/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanFilesListReader_H
#define PacmanFilesListReader_H

#include "pacmanprocessreader.h"
#include <QStringList>

class PacmanFilesListReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanFilesListReader(QObject *parent = 0);

protected:
    void send_parameters();
    QByteArray command() const;

signals:
    void files_ready(const QString & package,const QStringList & files);
};

#endif // PacmanFilesListReader_H
