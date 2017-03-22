/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanFilesListReader_H
#define PacmanFilesListReader_H

#include "pacmanprocessreader.h"
#include <QMap>
#include "libqpacman_global.h"

class LIBQPACMANSHARED_EXPORT PacmanFilesListReader : public PacmanProcessReader {
    Q_OBJECT
public:
    PacmanFilesListReader(const QString & package,QObject *parent = 0);
    QString package() const;

protected:
    QString command() const;
    bool output(const QString & out);

protected slots:
    void onFinished(int code,QProcess::ExitStatus status);

signals:
    void files_ready(const QStringList & files);

private:
    QString m_package;
    QStringList m_files;
    QString m_dir;
};

#endif // PacmanFilesListReader_H
