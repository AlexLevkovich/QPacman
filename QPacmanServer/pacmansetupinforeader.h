/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanSetupInfoReader_H
#define PacmanSetupInfoReader_H

#include "pacmanprocessreader.h"

class PacmanSetupInfoReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanSetupInfoReader(QObject *parent = 0);

protected slots:
    bool error(const QString & err);
    bool output(const QString & out);

protected slots:
    void onFinished(int code,QProcess::ExitStatus status);

protected:
    QString command() const;
};

#endif // PacmanSetupInfoReader_H
