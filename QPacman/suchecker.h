/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef SUCHECKER_H
#define SUCHECKER_H

#include "pacmanprocessreader.h"

class SuChecker : public PacmanProcessReader {
    Q_OBJECT
public:
    SuChecker(const QString & password,QObject *parent = 0);
    inline bool ok() const { return (m_ok != 0); }

protected:
    QString command() const;
    bool error(const QString & out);

protected slots:
    void onFinished(int code,QProcess::ExitStatus status);

private:
    int m_ok;
    QString m_password;
};

#endif // SUCHECKER_H
